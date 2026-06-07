#include "openssl_utils/ossl_http_client.h"

#include <android/log.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/http.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509_vfy.h>
#include <openssl/sha.h>

#include <algorithm>
#include <map>
#include <mutex>
#include <cctype>
#include <dirent.h>
#include <memory>
#include <vector>

#define LOG_TAG "OSSL_HttpClient"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace ossl_utils {

    namespace {

        static constexpr int kHttpBufferSize = 8192;
        static constexpr size_t kMaxResponseBytes = 1024 * 1024;

        struct BioDeleter {
            void operator()(BIO *bio) const {
                BIO_free_all(bio);
            }
        };

        struct SslCtxDeleter {
            void operator()(SSL_CTX *ctx) const {
                SSL_CTX_free(ctx);
            }
        };

        struct HttpTarget {
            std::string host;
            std::string port;
            std::string hostForSni;
            std::string pathAndQuery;
            std::string scheme;
        };

        struct TlsState {
            SSL_CTX *sslCtx;
            const char *host;
            bool verifyPeer;
        };

        std::string popOpenSslErrors() {
            std::string message;
            unsigned long err = 0;
            while ((err = ERR_get_error()) != 0) {
                char buf[256] = {0};
                ERR_error_string_n(err, buf, sizeof(buf));
                if (!message.empty()) {
                    message.append("; ");
                }
                message.append(buf);
            }
            return message;
        }

        std::string toLower(std::string value) {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
            return value;
        }

        int loadCaCertificatesFromDir(SSL_CTX *sslCtx, const char *dirPath) {
            if (sslCtx == nullptr || dirPath == nullptr) return 0;
            DIR *dir = opendir(dirPath);
            if (dir == nullptr) return 0;

            X509_STORE *store = SSL_CTX_get_cert_store(sslCtx);
            int loadedCount = 0;
            while (dirent *entry = readdir(dir)) {
                if (entry->d_name[0] == '.') continue;
                const std::string certPath = std::string(dirPath) + "/" + entry->d_name;
                BIO *certBio = BIO_new_file(certPath.c_str(), "r");
                if (certBio == nullptr) { ERR_clear_error(); continue; }
                while (true) {
                    X509 *cert = PEM_read_bio_X509(certBio, nullptr, nullptr, nullptr);
                    if (cert == nullptr) { ERR_clear_error(); break; }
                    if (X509_STORE_add_cert(store, cert) == 1) loadedCount++;
                    else ERR_clear_error();
                    X509_free(cert);
                }
                BIO_free(certBio);
            }
            closedir(dir);
            return loadedCount;
        }

        bool isIpv6Literal(const std::string &host) {
            return host.find(':') != std::string::npos;
        }

        bool parseUrlTarget(const std::string &url, HttpTarget *target) {
            if (target == nullptr) return false;
            const auto schemeEnd = url.find("://");
            if (schemeEnd == std::string::npos) return false;

            target->scheme = toLower(url.substr(0, schemeEnd));
            if (target->scheme != "http" && target->scheme != "https") return false;

            const auto authorityStart = schemeEnd + 3;
            const auto pathStart = url.find_first_of("/?#", authorityStart);
            std::string authority = pathStart == std::string::npos
                                    ? url.substr(authorityStart)
                                    : url.substr(authorityStart, pathStart - authorityStart);
            if (authority.empty() || authority.find('@') != std::string::npos) return false;

            const auto defaultPort = target->scheme == "https" ? "443" : "80";
            if (authority.front() == '[') {
                const auto bracketEnd = authority.find(']');
                if (bracketEnd == std::string::npos) return false;
                target->host = authority.substr(1, bracketEnd - 1);
                if (bracketEnd + 1 < authority.size() && authority[bracketEnd + 1] == ':') {
                    target->port = authority.substr(bracketEnd + 2);
                } else target->port = defaultPort;
            } else {
                const auto colon = authority.rfind(':');
                if (colon != std::string::npos) {
                    target->host = authority.substr(0, colon);
                    target->port = authority.substr(colon + 1);
                } else {
                    target->host = authority;
                    target->port = defaultPort;
                }
            }

            if (target->host.empty() || target->port.empty()) return false;
            target->hostForSni = isIpv6Literal(target->host) ? "" : target->host;

            if (pathStart == std::string::npos) target->pathAndQuery = "/";
            else {
                std::string pathAndQuery = url.substr(pathStart);
                const auto fragmentStart = pathAndQuery.find('#');
                if (fragmentStart != std::string::npos) pathAndQuery = pathAndQuery.substr(0, fragmentStart);
                if (pathAndQuery.empty() || pathAndQuery.front() == '#') target->pathAndQuery = "/";
                else if (pathAndQuery.front() == '?') target->pathAndQuery = "/" + pathAndQuery;
                else target->pathAndQuery = pathAndQuery;
            }
            return true;
        }

        // --- 反抓包逻辑 ---
        static std::map<std::string, std::string> g_dynamicPins;
        static std::mutex g_pinMutex;

        std::string getCertPublicKeySha256(X509 *cert) {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            unsigned int hash_len;
            if (X509_pubkey_digest(cert, EVP_sha256(), hash, &hash_len) != 1) return "";
            char hex[SHA256_DIGEST_LENGTH * 2 + 1];
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) sprintf(hex + i * 2, "%02x", hash[i]);
            return std::string(hex);
        }

        bool isSuspiciousIssuer(X509 *cert) {
            X509_NAME *issuer = X509_get_issuer_name(cert);
            char buf[512];
            if (X509_NAME_oneline(issuer, buf, sizeof(buf))) {
                std::string s(buf);
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                if (s.find("charles") != std::string::npos || s.find("fiddler") != std::string::npos ||
                    s.find("proxy") != std::string::npos || s.find("canary") != std::string::npos ||
                    s.find("mitm") != std::string::npos) return true;
            }
            return false;
        }

        int verifyCallback(int preverify_ok, X509_STORE_CTX *x509_ctx) {
            if (!preverify_ok) return 0;
            X509 *cert = X509_STORE_CTX_get_current_cert(x509_ctx);
            if (cert == nullptr) return 0;
            if (isSuspiciousIssuer(cert)) {
                LOGD("Anti-Sniffing: Suspicious Proxy Issuer detected!");
                return 0;
            }
            int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
            if (depth == 0) {
                SSL *ssl = static_cast<SSL *>(X509_STORE_CTX_get_ex_data(x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
                const char *hostname = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
                if (hostname) {
                    std::string hostStr(hostname);
                    std::string currentHash = getCertPublicKeySha256(cert);
                    std::lock_guard<std::mutex> lock(g_pinMutex);
                    if (g_dynamicPins.find(hostStr) == g_dynamicPins.end()) {
                        g_dynamicPins[hostStr] = currentHash;
                    } else if (g_dynamicPins[hostStr] != currentHash) {
                        LOGD("Anti-Sniffing: Certificate hash changed for [%s]! Possible MITM attack.", hostname);
                        return 0;
                    }
                }
            }
            return 1;
        }

        BIO *httpBioUpdate(BIO *bio, void *arg, int connect, int detail) {
            if (connect == 0) {
                if (bio != nullptr && detail != 0) {
                    BIO_ssl_shutdown(bio);
                    BIO *plainBio = BIO_pop(bio);
                    BIO_free(bio);
                    return plainBio;
                }
                return bio;
            }
            auto *tlsState = static_cast<TlsState *>(arg);
            if (tlsState == nullptr || tlsState->sslCtx == nullptr) return bio;
            BIO *sslBio = BIO_new_ssl(tlsState->sslCtx, 1);
            if (sslBio == nullptr) return nullptr;
            SSL *ssl = nullptr;
            BIO_get_ssl(sslBio, &ssl);
            if (ssl != nullptr && tlsState->host != nullptr && tlsState->host[0] != '\0') {
                SSL_set_tlsext_host_name(ssl, tlsState->host);
                if (tlsState->verifyPeer) SSL_set1_host(ssl, tlsState->host);
            }
            BIO_push(sslBio, bio);
            return sslBio;
        }

        std::unique_ptr<SSL_CTX, SslCtxDeleter> createSslCtx(bool verifyPeer) {
            std::unique_ptr<SSL_CTX, SslCtxDeleter> sslCtx(SSL_CTX_new(TLS_client_method()));
            if (!sslCtx) {
                LOGD("SSL_CTX_new failed: %s", popOpenSslErrors().c_str());
                return nullptr;
            }
            if (!verifyPeer) {
                SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_NONE, nullptr);
                return sslCtx;
            }
            SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_PEER, verifyCallback);
            loadCaCertificatesFromDir(sslCtx.get(), "/system/etc/security/cacerts");
            loadCaCertificatesFromDir(sslCtx.get(), "/apex/com.android.conscrypt/cacerts");
            return sslCtx;
        }

        std::string readAllFromBio(BIO *bio) {
            std::string data;
            char buffer[kHttpBufferSize] = {0};
            while (true) {
                const int readCount = BIO_read(bio, buffer, sizeof(buffer));
                if (readCount > 0) { data.append(buffer, static_cast<size_t>(readCount)); continue; }
                if (readCount == 0 || !BIO_should_retry(bio)) break;
            }
            return data;
        }

        std::string parseRawHttpBody(const std::string &rawResponse) {
            const auto statusEnd = rawResponse.find("\r\n");
            if (statusEnd == std::string::npos) return "";
            const std::string statusLine = rawResponse.substr(0, statusEnd);
            if (statusLine.size() < 12 || statusLine.substr(0, 5) != "HTTP/") return "";
            int statusCode = 0;
            try { statusCode = std::stoi(statusLine.substr(9, 3)); } catch (...) { return ""; }
            if (statusCode < 200 || statusCode >= 300) return "";
            const auto bodyStart = rawResponse.find("\r\n\r\n");
            if (bodyStart == std::string::npos) return "";
            return rawResponse.substr(bodyStart + 4);
        }

        std::string fetchByRawBio(const HttpTarget &target, bool verifyPeer) {
            std::unique_ptr<BIO, BioDeleter> conn;
            std::unique_ptr<SSL_CTX, SslCtxDeleter> sslCtx;
            const std::string hostserv = isIpv6Literal(target.host) ? "[" + target.host + "]:" + target.port : target.host + ":" + target.port;
            if (target.scheme == "https") {
                sslCtx = createSslCtx(verifyPeer);
                if (!sslCtx) return "";
                conn.reset(BIO_new_ssl_connect(sslCtx.get()));
                if (!conn) return "";
                SSL *ssl = nullptr;
                BIO_get_ssl(conn.get(), &ssl);
                if (ssl != nullptr && !target.hostForSni.empty()) {
                    SSL_set_tlsext_host_name(ssl, target.hostForSni.c_str());
                    if (verifyPeer) SSL_set1_host(ssl, target.hostForSni.c_str());
                }
            } else {
                conn.reset(BIO_new_connect(hostserv.c_str()));
                if (!conn) return "";
            }
            BIO_set_conn_hostname(conn.get(), hostserv.c_str());
            if (BIO_do_connect(conn.get()) <= 0) return "";
            const std::string request = "GET " + target.pathAndQuery + " HTTP/1.1\r\nHost: " +
                                       (isIpv6Literal(target.host) ? "[" + target.host + "]" : target.host) +
                                       (target.port == "80" || target.port == "443" ? "" : ":" + target.port) +
                                       "\r\nUser-Agent: okhttp/5.1.0\r\nAccept: */*\r\nConnection: close\r\n\r\n";
            if (BIO_write(conn.get(), request.data(), static_cast<int>(request.size())) <= 0 || BIO_flush(conn.get()) <= 0) return "";
            return parseRawHttpBody(readAllFromBio(conn.get()));
        }

        std::string fetchByOpenSsl(const std::string &url, bool verifyPeer, int timeout) {
            HttpTarget target;
            if (!parseUrlTarget(url, &target)) return "";
            std::unique_ptr<SSL_CTX, SslCtxDeleter> sslCtx;
            TlsState tlsState{nullptr, nullptr, verifyPeer};
            OSSL_HTTP_bio_cb_t bioUpdate = nullptr;
            void *bioUpdateArg = nullptr;
            if (target.scheme == "https") {
                sslCtx = createSslCtx(verifyPeer);
                if (!sslCtx) return "";
                tlsState.sslCtx = sslCtx.get();
                tlsState.host = target.hostForSni.c_str();
                bioUpdate = httpBioUpdate;
                bioUpdateArg = &tlsState;
            }
            std::unique_ptr<BIO, BioDeleter> response(OSSL_HTTP_get(url.c_str(), nullptr, nullptr, nullptr, nullptr,
                                                                  bioUpdate, bioUpdateArg, kHttpBufferSize, nullptr, nullptr, 0, kMaxResponseBytes, timeout));
            if (!response) return "";
            return readAllFromBio(response.get());
        }
    }

    std::string fetch_http_data(const std::string &url, int timeout_seconds) {
        HttpTarget target;
        if (!parseUrlTarget(url, &target)) return "";
        std::string result = fetchByOpenSsl(url, true, timeout_seconds);
        if (result.empty()) result = fetchByRawBio(target, true);
        return result;
    }
}
