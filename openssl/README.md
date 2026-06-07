# OpenSSL Android 接入与反抓包指南

本项目演示了如何在 Android NDK 工程中接入预编译的 OpenSSL 静态库，实现 Native 层网络请求并内置反抓包（Anti-Sniffing）防护。

---

## 1. 如何在其他模块中使用

如果你需要在其他 Native 模块（如 `app` 模块的 C++ 代码）中调用 OpenSSL 或封装好的工具类，请遵循以下步骤：

### 1.1 CMake 配置
在你的模块的 `CMakeLists.txt` 中引入并链接：

```cmake
# 1. 引入 openssl 子项目（路径需根据实际目录层级调整）
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/path/to/openssl openssl)

# 2. 链接 OpenSSL::Utils
# 该库包含了封装好的 HTTP 客户端及核心 OpenSSL (SSL/Crypto) 静态库
target_link_libraries(your_target_name 
    OpenSSL::Utils 
    log
)
```

### 1.2 C++ 代码引用
由于 `OpenSSL::Utils` 已经配置了头文件包含路径，你可以直接引用：

```cpp
// 引用本项目封装的 HTTP 工具类
#include "openssl_utils/ossl_http_client.h"

// 引用原始 OpenSSL API（如果需要直接操作底层）
#include <openssl/ssl.h>
#include <openssl/err.h>

void example() {
    // 调用封装好的 HTTPS 请求方法（内部自带反抓包校验）
    std::string data = ossl_utils::fetch_http_data("https://api.example.com/data", 10);
}
```

---

## 2. Native 层 HTTP/HTTPS 请求实现

`ossl_http_client.cpp` 提供了两种实现方式：一种是利用 OpenSSL 3.x 高级 API，另一种是基于 BIO 的底层实现作为兼容方案。

### 2.1 使用 OSSL_HTTP_get 实现（高级 API）
这是最便捷的方式，OpenSSL 内部封装了 HTTP 协议的处理。

```cpp
std::unique_ptr<BIO, BioDeleter> response(OSSL_HTTP_get(
    url.c_str(),      // 请求地址
    nullptr, nullptr, // 代理与主机信息
    nullptr, nullptr, // 端口与路径
    bioUpdate,        // 关键：用于 HTTPS 握手升级的回调
    bioUpdateArg,     // 回调参数（包含 SSL_CTX）
    kHttpBufferSize,  // 缓冲区大小
    nullptr, nullptr, // 头部信息
    0,                // 标志位
    kMaxResponseBytes,// 最大响应长度
    timeout           // 超时时间
));
```

### 2.2 基于 BIO 的底层实现（Fallback 方案）
当高级 API 不可用时，通过 `BIO_new_ssl_connect` 手动构建请求栈。

```cpp
// 1. 创建连接 BIO
conn.reset(BIO_new_ssl_connect(sslCtx.get()));

// 2. 设置 SNI (Server Name Indication)
SSL_set_tlsext_host_name(ssl, target.hostForSni.c_str());

// 3. 执行 TCP + TLS 握手
BIO_do_connect(conn.get());

// 4. 手动构造 HTTP 报文并发送
std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
BIO_write(conn.get(), request.data(), request.size());

// 5. 循环读取响应数据
BIO_read(conn.get(), buffer, sizeof(buffer));
```

---

## 3. 反抓包（Anti-Sniffing）实现原理

反抓包的核心思路是在 TLS 握手阶段对证书进行严格审查，防止中间人代理（如 Charles/Fiddler）介入。

### 3.1 强制 SSL 验证与证书隔离
在创建 `SSL_CTX` 时，不仅开启 `SSL_VERIFY_PEER`，还手动加载系统分区的根证书，从而无视用户手动安装的代理 CA 证书。

```cpp
std::unique_ptr<SSL_CTX, SslCtxDeleter> createSslCtx(bool verifyPeer) {
    auto sslCtx = std::unique_ptr<SSL_CTX, SslCtxDeleter>(SSL_CTX_new(TLS_client_method()));
    
    // 注册自定义验证回调
    SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_PEER, verifyCallback);
    
    // 只加载系统预设的可信证书路径，隔离用户自行安装的风险证书
    loadCaCertificatesFromDir(sslCtx.get(), "/system/etc/security/cacerts");
    return sslCtx;
}
```

### 3.2 抓包工具证书指纹检测
在 `verifyCallback` 过程中，通过 `isSuspiciousIssuer` 扫描证书发行者，拦截常见的代理软件证书。

```cpp
bool isSuspiciousIssuer(X509 *cert) {
    X509_NAME *issuer = X509_get_issuer_name(cert);
    char buf[512];
    X509_NAME_oneline(issuer, buf, sizeof(buf));
    
    std::string s = toLower(buf);
    // 拦截包含代理关键字的发行者
    if (s.find("charles") != std::string::npos || 
        s.find("fiddler") != std::string::npos ||
        s.find("canary")  != std::string::npos) {
        return true;
    }
    return false;
}
```

### 3.3 动态公钥固定 (Dynamic Pinning)
在内存中缓存域名的公钥哈希（SHA-256）。利用 `X509_pubkey_digest` 获取公钥指纹，并在内存中通过 `std::map` 维护一张动态白名单。

```cpp
// 获取当前证书的公钥 SHA-256 哈希
std::string currentHash = getCertPublicKeySha256(cert);

// 动态 Pinning 校验逻辑
if (g_dynamicPins.find(host) == g_dynamicPins.end()) {
    g_dynamicPins[host] = currentHash; // 首次记录
} else if (g_dynamicPins[host] != currentHash) {
    // 同一域名公钥指纹不一致，疑似遭遇 MITM 攻击
    return 0; // 终止握手
}
```

## 4. 为什么这种方式更安全？

1.  **绕过 Java 系统代理**：由于请求在 Native 层发起，Android 系统的“Wi-Fi 代理设置”无法捕获此流量。
2.  **免疫常规 Hook**：由于代码静态链接了 `libssl.a`，不依赖系统 `libssl.so`，因此像 SSLUnpinning 等基于劫持系统动态库函数的工具会直接失效。
3.  **不信任用户证书**：通过代码层面限定证书加载路径，即使手机 root 后在用户区安装了代理证书也无法生效。


OpenSSL 反抓包 ASCII 流程图：
```markdown
       [ App 发起请求 ]
              |
    +---------v-------------------------+
    |  1. 初始化 SSL_CTX                |
    |  ( 只从 /system 加载根证书 )       | <---【第一层：证书隔离】
    +---------+-------------------------+
              |
    +---------v-------------------------+
    |  2. 注册 verifyCallback           | <---【逻辑钩子入口】
    +---------+-------------------------+
              |
    +---------v-------------------------+
    |  3. 执行 BIO_do_connect (握手)    | <---【TLS Handshake】
    +---------+-------------------------+
              |
              | ( 握手阶段：证书校验回调 )
              v
    +-----------------------------------+
    |        verifyCallback()           |
    |                                   |
    |  [A] 检查 Issuer 字段              | <---【第二层：指纹识别】
    |      ( 匹配 "Charles/Fiddler"? )  |----------+
    |                                   |          |
    |  [B] 提取公钥并计算 SHA256         |          | [匹配/异常]
    |                                   |          |
    |  [C] 动态公钥固定校验              | <---【第三层：公钥固定】
    |      ( 哈希一致性比对? )           |          |
    +-----------------+-----------------+          |
                      |                            |
            [ 校验通过: 返回 1 ]            [ 校验失败: 返回 0 ]
                      |                            |
    +-----------------v-----------------+          |
    |  4. 握手完成，建立加密隧道          |          |
    |                                   |          v
    |  [D] 绕过 Java 系统代理            |    +-----------+
    |  [E] 绕过 Java 层 Hook            |    | 连接中止  |
    |                                   |    | (抓包失败)|
    +-----------------v-----------------+    +-----------+
                      |
              [ 安全发送/接收数据 ]
```

