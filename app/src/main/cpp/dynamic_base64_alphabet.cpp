#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>

#define TAG "DynamicBase64"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// 标准 Base64 码表
static const std::string DYNAMIC_BASE64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 根据字符串长度生成 Base64 动态码表
__attribute__((annotate("bcf"))) std::string generateDynamicBase64Alphabet(size_t length) {
    std::string dynamicBase64(64, ' ');
    int xor_key = length % 64;

    for (int i = 0; i < 64; i++) {
        int new_index = i ^ xor_key;  // 计算新的索引
        dynamicBase64[new_index] = DYNAMIC_BASE64_ALPHABET[i];  // 重新排列字符
    }

    return dynamicBase64;
}



// 动态 Base64 编码
__attribute__((annotate("bcf"))) std::string dynamicBase64Encode(const uint8_t* data, size_t length) {
    std::string base64Alphabet = generateDynamicBase64Alphabet(length);
    std::string encoded;
    int val = 0, valb = -6;

    for (size_t i = 0; i < length; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64Alphabet[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(base64Alphabet[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    return encoded;
}


// 动态 Base64 解码
__attribute__((annotate("bcf"))) std::vector<uint8_t> dynamicBase64Decode(const std::string& input, size_t originalLength) {
    std::string base64Alphabet = generateDynamicBase64Alphabet(originalLength);
    std::vector<int> T(256, -1);

    for (int i = 0; i < 64; i++) {
        T[base64Alphabet[i]] = i;  // 生成解码表
    }

    std::vector<uint8_t> decoded;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return decoded;
}


// JNI 动态 Base64 编码
extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_base64_Base64Activity_dynamicBase64Encode(JNIEnv* env, jobject, jbyteArray input) {
    jsize length = env->GetArrayLength(input);
    jbyte* data = env->GetByteArrayElements(input, nullptr);

    std::string encoded = dynamicBase64Encode(reinterpret_cast<uint8_t*>(data), length);

    env->ReleaseByteArrayElements(input, data, JNI_ABORT);
    return env->NewStringUTF(encoded.c_str());
}

// JNI 动态 Base64 解码
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_cyrus_example_base64_Base64Activity_dynamicBase64Decode(JNIEnv* env, jobject, jstring input, jint originalLength) {
    const char* encodedStr = env->GetStringUTFChars(input, nullptr);
    std::vector<uint8_t> decoded = dynamicBase64Decode(std::string(encodedStr), originalLength);
    env->ReleaseStringUTFChars(input, encodedStr);

    jbyteArray output = env->NewByteArray(decoded.size());
    env->SetByteArrayRegion(output, 0, decoded.size(), reinterpret_cast<jbyte*>(decoded.data()));
    return output;
}

