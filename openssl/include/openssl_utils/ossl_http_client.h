#ifndef OSSL_HTTP_CLIENT_H
#define OSSL_HTTP_CLIENT_H

#include <string>

/**
 * 使用 OpenSSL 执行 HTTP GET 请求的封装接口。
 * 这个文件所在的模块由 library 和 app 共享。
 */

namespace ossl_utils {

    /**
     * 获取 HTTP 数据。
     * 内部会自动尝试高级 OSSL_HTTP API 和底层的 Raw BIO 模式。
     * 支持自动反抓包 (TOFU + 启发式检测)。
     *
     * @param url 完整的请求地址 (https://...)
     * @param timeout_seconds 超时时间（秒）
     * @return 成功返回 Body 内容，失败返回空字符串。
     */
    std::string fetch_http_data(const std::string &url, int timeout_seconds = 10);

}

#endif // OSSL_HTTP_CLIENT_H
