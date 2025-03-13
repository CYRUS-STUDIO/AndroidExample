#include <jni.h>
#include <string>
#include <android/log.h>

#define TAG "CustomCRC32"

unsigned int custom_crc32_table[256];

// 初始化 CRC32 查找表
void init_custom_crc32_table() {
    unsigned int crc;
    for (int i = 0; i < 256; i++) {
        crc = i;
        for (int j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xd76aa478;
            } else {
                crc >>= 1;
            }
        }
        custom_crc32_table[i] = crc;
    }
}

// 计算 CRC32
unsigned int custom_crc32(const std::string& str) {
    unsigned int crc = 0xffffffff;
    for (size_t i = 0; i < str.size(); i++) {
        unsigned char byte = str[i];
        crc = (crc >> 8) ^ custom_crc32_table[(crc ^ byte) & 0xff];
    }
    return ~crc;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_crc32_CRC32Utils_customTableCRC32(JNIEnv *env, jclass , jstring input) {
    // 初始化 CRC32 表（只需调用一次）
    static bool is_initialized = false;
    if (!is_initialized) {
        init_custom_crc32_table();
        is_initialized = true;
    }

    // 获取输入的字符串
    const char* str = env->GetStringUTFChars(input, nullptr);
    std::string input_str(str);
    env->ReleaseStringUTFChars(input, str);

    // 计算 CRC32 值
    unsigned int crc = custom_crc32(input_str);

    // 返回 CRC32 的十六进制字符串
    char crcHex[9];  // CRC32 是 32 位，最大为 8 位十六进制数
    snprintf(crcHex, sizeof(crcHex), "%08x", crc);
    return env->NewStringUTF(crcHex);
}
