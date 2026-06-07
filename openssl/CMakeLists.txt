cmake_minimum_required(VERSION 3.22.1)

project(openssl_prebuilt)

# 定义 OpenSSL 根目录（当前目录）
set(OPENSSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(OPENSSL_ABI_ROOT "${OPENSSL_ROOT}/${ANDROID_ABI}")

message(STATUS "Loading OpenSSL from: ${OPENSSL_ABI_ROOT}")

# 校验头文件
if (NOT EXISTS "${OPENSSL_ABI_ROOT}/include/openssl/ssl.h")
    message(FATAL_ERROR "OpenSSL headers not found for ${ANDROID_ABI} at: ${OPENSSL_ABI_ROOT}/include")
endif ()

# 校验库文件
if (NOT EXISTS "${OPENSSL_ABI_ROOT}/lib/libssl.a")
    message(FATAL_ERROR "OpenSSL libssl.a not found for ${ANDROID_ABI} at: ${OPENSSL_ABI_ROOT}/lib/libssl.a")
endif ()

if (NOT EXISTS "${OPENSSL_ABI_ROOT}/lib/libcrypto.a")
    message(FATAL_ERROR "OpenSSL libcrypto.a not found for ${ANDROID_ABI} at: ${OPENSSL_ABI_ROOT}/lib/libcrypto.a")
endif ()

# 定义 SSL 静态库
add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_ABI_ROOT}/lib/libssl.a"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_ABI_ROOT}/include"
)

# 定义 Crypto 静态库
add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
set_target_properties(OpenSSL::Crypto PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_ABI_ROOT}/lib/libcrypto.a"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_ABI_ROOT}/include"
)

# --- OpenSSL 通用工具库 ---
# 这是一个可编译的库，包装了常用的 OpenSSL 操作，如 HTTP 请求和反抓包
add_library(openssl_utils STATIC ossl_http_client.cpp)

target_include_directories(openssl_utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(openssl_utils
    PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        log
)

# 使 openssl_utils 在外部模块（如 library 和 app）中可用
add_library(OpenSSL::Utils ALIAS openssl_utils)
