#include <jni.h>
#include <string>
#include "sha1.h"

// 将字节数组转换为十六进制字符串
std::string bytesToHex(const unsigned char* bytes, size_t length) {
    std::string hex;
    const char hexDigits[] = "0123456789abcdef";
    for (size_t i = 0; i < length; i++) {
        hex += hexDigits[(bytes[i] >> 4) & 0x0F];
        hex += hexDigits[bytes[i] & 0x0F];
    }
    return hex;
}


// JNI 方法实现
extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_sha1_SHA1Utils_sha1(JNIEnv* env, jclass clazz, jstring input) {
    const char* inputStr = env->GetStringUTFChars(input, nullptr);
    if (!inputStr) {
        return nullptr; // 内存分配失败
    }

    // 初始化 SHA1 上下文
    SHA1_CTX ctx;
    unsigned char digest[SHA1_MAC_LEN];

    SHA1Init(&ctx);
    SHA1Update(&ctx, (const unsigned char*)inputStr, (u32)strlen(inputStr));
    SHA1Final(digest, &ctx);

    env->ReleaseStringUTFChars(input, inputStr);

    // 转换为 hex 字符串
    std::string hexResult = bytesToHex(digest, SHA1_MAC_LEN);

    // 返回结果
    return env->NewStringUTF(hexResult.c_str());
}


// JNI 方法实现
extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_sha1_SHA1Utils_changeSHA1Init(JNIEnv* env, jclass clazz, jstring input) {
    const char* inputStr = env->GetStringUTFChars(input, nullptr);
    if (!inputStr) {
        return nullptr; // 内存分配失败
    }

    // 初始化 SHA1 上下文
    SHA1_CTX ctx;
    unsigned char digest[SHA1_MAC_LEN];

    SHA1Init2(&ctx);
    SHA1Update(&ctx, (const unsigned char*)inputStr, (u32)strlen(inputStr));
    SHA1Final(digest, &ctx);

    env->ReleaseStringUTFChars(input, inputStr);

    // 转换为 hex 字符串
    std::string hexResult = bytesToHex(digest, SHA1_MAC_LEN);

    // 返回结果
    return env->NewStringUTF(hexResult.c_str());
}



// JNI 方法实现
extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_sha1_SHA1Utils_changeSHA1Update(JNIEnv* env, jclass clazz, jstring input) {
    const char* inputStr = env->GetStringUTFChars(input, nullptr);
    if (!inputStr) {
        return nullptr; // 内存分配失败
    }

    // 初始化 SHA1 上下文
    SHA1_CTX ctx;
    unsigned char digest[SHA1_MAC_LEN];

    SHA1Init(&ctx);
    SHA1Update(&ctx, (const unsigned char*)"cyrus", (u32)strlen(inputStr));
    SHA1Update(&ctx, (const unsigned char*)inputStr, (u32)strlen(inputStr));
    SHA1Update(&ctx, (const unsigned char*)"studio", (u32)strlen(inputStr));
    SHA1Final(digest, &ctx);

    env->ReleaseStringUTFChars(input, inputStr);

    // 转换为 hex 字符串
    std::string hexResult = bytesToHex(digest, SHA1_MAC_LEN);

    // 返回结果
    return env->NewStringUTF(hexResult.c_str());
}


// JNI 方法实现
extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_sha1_SHA1Utils_changeConstant(JNIEnv* env, jclass clazz, jstring input) {
    const char* inputStr = env->GetStringUTFChars(input, nullptr);
    if (!inputStr) {
        return nullptr; // 内存分配失败
    }

    // 初始化 SHA1 上下文
    SHA1_CTX ctx;
    unsigned char digest[SHA1_MAC_LEN];

    SHA1Init3(&ctx);
    SHA1Update3(&ctx, (const unsigned char*)inputStr, (u32)strlen(inputStr));
    SHA1Final3(digest, &ctx);

    env->ReleaseStringUTFChars(input, inputStr);

    // 转换为 hex 字符串
    std::string hexResult = bytesToHex(digest, SHA1_MAC_LEN);

    // 返回结果
    return env->NewStringUTF(hexResult.c_str());
}
