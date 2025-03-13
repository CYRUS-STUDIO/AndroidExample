#include <jni.h>
#include <string>
#include <android/log.h>

#define TAG "ModifiedCRC32"

// 魔改版 CRC32 表
unsigned int modified_crc32_table[256];

// 初始化魔改版 CRC32 查找表
void init_modified_crc32_table() {
    for (int i = 0; i < 256; i++) {
        unsigned int crc = i;
        for (int j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0x82F63B78;
            } else {
                crc >>= 1;
            }
        }
        modified_crc32_table[i] = crc;
    }
}

// 魔改版 CRC32 计算
unsigned int modified_crc32(const std::string& str) {
    unsigned int crc = 0xDEADBEEF;  // 魔改初始值
    for (size_t i = 0; i < str.size(); i++) {
        unsigned char byte = str[i] ^ 0xA5;  // 额外扰动
        crc = ((crc >> 7) ^ modified_crc32_table[(crc ^ byte) & 0xff]) ^ 0xA5A5A5A5; // 变更位移方式 & 额外 XOR 干扰
    }
    return ~crc;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_crc32_CRC32Utils_modifiedCRC32(JNIEnv *env, jclass, jstring input) {
    // 初始化 CRC32 表（只需调用一次）
    static bool is_initialized = false;
    if (!is_initialized) {
        init_modified_crc32_table();
        is_initialized = true;
    }

    // 获取输入字符串
    const char* str = env->GetStringUTFChars(input, nullptr);
    std::string input_str(str);
    env->ReleaseStringUTFChars(input, str);

    // 计算魔改版 CRC32
    unsigned int crc = modified_crc32(input_str);

    // 返回十六进制格式的 CRC32 结果
    char crcHex[9];
    snprintf(crcHex, sizeof(crcHex), "%08x", crc);
    return env->NewStringUTF(crcHex);
}
