#include <jni.h>
#include <string>
#include <android/log.h>
#include "base64.h"

#define TAG "Base64"

static const std::string BASE64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 检查字符是否是 Base64 字符
static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

// Base64 编码函数
std::string base64_encode(const unsigned char *bytes_to_encode, int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                ret += BASE64_ALPHABET[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; j++) {
            ret += BASE64_ALPHABET[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

// Base64 解码函数
std::string base64_decode(std::string const &encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = BASE64_ALPHABET.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                ret += char_array_3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (int j = 0; j < 4; j++) {
            char_array_4[j] = BASE64_ALPHABET.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int j = 0; j < i - 1; j++) {
            ret += char_array_3[j];
        }
    }

    return ret;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_base64_Base64Activity_nativeBase64Encode(JNIEnv *env, jobject, jbyteArray input) {
    jsize input_len = env->GetArrayLength(input);
    unsigned char *bytes_to_encode = reinterpret_cast<unsigned char *>(env->GetByteArrayElements(input, nullptr));

    std::string encoded = base64_encode(bytes_to_encode, input_len);

    env->ReleaseByteArrayElements(input, reinterpret_cast<jbyte *>(bytes_to_encode), 0);
    return env->NewStringUTF(encoded.c_str());
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_cyrus_example_base64_Base64Activity_nativeBase64Decode(JNIEnv *env, jobject, jstring input) {
    const char *encoded_chars = env->GetStringUTFChars(input, nullptr);
    std::string encoded_string(encoded_chars);

    std::string decoded_string = base64_decode(encoded_string);

    env->ReleaseStringUTFChars(input, encoded_chars);

    jbyteArray decoded_byte_array = env->NewByteArray(decoded_string.size());
    env->SetByteArrayRegion(decoded_byte_array, 0, decoded_string.size(), reinterpret_cast<const jbyte *>(decoded_string.c_str()));

    return decoded_byte_array;
}
