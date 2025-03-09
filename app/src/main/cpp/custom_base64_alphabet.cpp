#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>

#define LOG_TAG "CustomBase64"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// 自定义 Base64 码表
static const std::string CUSTOM_BASE64_ALPHABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";

// **自定义 Base64 编码**
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_base64_Base64Activity_customBase64Encode(JNIEnv *env, jobject thiz, jbyteArray data) {
    jsize len = env->GetArrayLength(data);
    std::vector<unsigned char> input(len);
    env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte *>(input.data()));

    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(CUSTOM_BASE64_ALPHABET[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(CUSTOM_BASE64_ALPHABET[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    return env->NewStringUTF(encoded.c_str());
}

// **自定义 Base64 解码**
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_cyrus_example_base64_Base64Activity_customBase64Decode(JNIEnv *env, jobject thiz, jstring encodedStr) {
    const char *encoded = env->GetStringUTFChars(encodedStr, nullptr);
    if (encoded == nullptr) return nullptr; // 内存分配失败

    std::string input(encoded);
    env->ReleaseStringUTFChars(encodedStr, encoded);

    std::vector<unsigned char> output;
    std::vector<int> T(256, -1);
    for (int i = 0; i < CUSTOM_BASE64_ALPHABET.size(); i++) {
        T[CUSTOM_BASE64_ALPHABET[i]] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            output.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }

    jbyteArray result = env->NewByteArray(output.size());
    env->SetByteArrayRegion(result, 0, output.size(), reinterpret_cast<jbyte *>(output.data()));
    return result;
}
