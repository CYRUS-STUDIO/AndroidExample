#include <jni.h>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <sstream>
#include "sha256.h"

// 定义块大小和输出长度
constexpr size_t BLOCK_SIZE = 64;
constexpr size_t SHA256_DIGEST_LENGTH = 32;

// 将字符串转换为字节数组
extern std::vector<uint8_t> toBytes(const std::string& str);

// 将字节数组转换为十六进制字符串
extern std::string bytesToHex(const std::vector<uint8_t>& bytes);

// HMAC-SHA256 实现
void hmacSha256(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data, uint8_t* outDigest) {
    std::vector<uint8_t> modifiedKey = key;

    // 1. 密钥处理
    if (modifiedKey.size() > BLOCK_SIZE) {
        uint8_t hash[SHA256_DIGEST_LENGTH];
        SHA256_hash(modifiedKey.data(), modifiedKey.size(), hash);
        modifiedKey.assign(hash, hash + SHA256_DIGEST_LENGTH);
    }
    if (modifiedKey.size() < BLOCK_SIZE) {
        modifiedKey.resize(BLOCK_SIZE, 0x00); // 补零
    }

    // 2. 生成 ipad 和 opad
    std::vector<uint8_t> ipad(BLOCK_SIZE, 0x36);
    std::vector<uint8_t> opad(BLOCK_SIZE, 0x5c);

    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        ipad[i] ^= modifiedKey[i];
        opad[i] ^= modifiedKey[i];
    }

    // 3. Inner Hash: SHA256(ipad + data)
    uint8_t innerDigest[SHA256_DIGEST_LENGTH];
    SHA256_CTX innerCtx;
    SHA256_init(&innerCtx);
    SHA256_update(&innerCtx, ipad.data(), BLOCK_SIZE);
    SHA256_update(&innerCtx, data.data(), data.size());
    memcpy(innerDigest, SHA256_final(&innerCtx), SHA256_DIGEST_LENGTH);

    // 4. Outer Hash: SHA256(opad + Inner Hash)
    SHA256_CTX outerCtx;
    SHA256_init(&outerCtx);
    SHA256_update(&outerCtx, opad.data(), BLOCK_SIZE);
    SHA256_update(&outerCtx, innerDigest, SHA256_DIGEST_LENGTH);
    memcpy(outDigest, SHA256_final(&outerCtx), SHA256_DIGEST_LENGTH);
}

// JNI 接口实现
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_hmac_HMACUtils_hmacSHA256(
        JNIEnv* env,
        jclass,
        jstring data) {

    // 将 jstring 转换为 std::string
    const char* dataStr = env->GetStringUTFChars(data, nullptr);
    const char* keyStr = "CYRUS STUDIO";

    std::vector<uint8_t> dataBytes = toBytes(dataStr);
    std::vector<uint8_t> keyBytes = toBytes(keyStr);

    // 计算 HMAC-SHA256
    uint8_t resultDigest[SHA256_DIGEST_LENGTH];
    hmacSha256(keyBytes, dataBytes, resultDigest);

    // 转换结果为十六进制字符串
    std::string hexResult = bytesToHex(std::vector<uint8_t>(resultDigest, resultDigest + SHA256_DIGEST_LENGTH));

    // 释放资源
    env->ReleaseStringUTFChars(data, dataStr);

    return env->NewStringUTF(hexResult.c_str());
}
