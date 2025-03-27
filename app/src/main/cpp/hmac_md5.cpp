#include <jni.h>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <sstream>
#include "md5.h"

// 定义块大小和输出长度
constexpr size_t BLOCK_SIZE = 64;
constexpr size_t MD5_DIGEST_LENGTH = 16;

// 将字符串转换为字节数组
std::vector<uint8_t> toBytes(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

// 将字节数组转换为十六进制字符串
std::string bytesToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    for (uint8_t byte : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    return oss.str();
}

// HMAC-MD5 实现
void hmacMd5(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data, uint8_t* outDigest) {
    std::vector<uint8_t> modifiedKey = key;

    // 1. 密钥处理
    if (modifiedKey.size() > BLOCK_SIZE) {
        uint8_t hash[MD5_DIGEST_LENGTH];
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, modifiedKey.data(), modifiedKey.size());
        MD5_Final(hash, &ctx);
        modifiedKey.assign(hash, hash + MD5_DIGEST_LENGTH);
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

    // 3. Inner Hash: MD5(ipad + data)
    uint8_t innerDigest[MD5_DIGEST_LENGTH];
    MD5_CTX innerCtx;
    MD5_Init(&innerCtx);
    MD5_Update(&innerCtx, ipad.data(), BLOCK_SIZE);
    MD5_Update(&innerCtx, data.data(), data.size());
    MD5_Final(innerDigest, &innerCtx);

    // 4. Outer Hash: MD5(opad + Inner Hash)
    MD5_CTX outerCtx;
    MD5_Init(&outerCtx);
    MD5_Update(&outerCtx, opad.data(), BLOCK_SIZE);
    MD5_Update(&outerCtx, innerDigest, MD5_DIGEST_LENGTH);
    MD5_Final(outDigest, &outerCtx);
}

// JNI 接口实现
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_hmac_HMACUtils_hmacMD5(
        JNIEnv* env,
        jclass,
        jstring data) {

    // 将 jstring 转换为 std::string
    const char* dataStr = env->GetStringUTFChars(data, nullptr);
    const char* keyStr = "CYRUS STUDIO";

    std::vector<uint8_t> dataBytes = toBytes(dataStr);
    std::vector<uint8_t> keyBytes = toBytes(keyStr);

    // 计算 HMAC-MD5
    uint8_t resultDigest[MD5_DIGEST_LENGTH];
    hmacMd5(keyBytes, dataBytes, resultDigest);

    // 转换结果为十六进制字符串
    std::string hexResult = bytesToHex(std::vector<uint8_t>(resultDigest, resultDigest + MD5_DIGEST_LENGTH));

    // 释放资源
    env->ReleaseStringUTFChars(data, dataStr);

    return env->NewStringUTF(hexResult.c_str());
}
