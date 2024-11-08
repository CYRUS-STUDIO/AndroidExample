#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "assembly-lib.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_assembly_AssemblyActivity_encryptString(JNIEnv *env, jobject /* this */,
                                                               jstring input) {
    const char *inputStr = env->GetStringUTFChars(input, nullptr);
    std::string encryptedStr(inputStr);

    // 获取输入字符串的 Unicode 码点
    const jchar *inputChars = env->GetStringChars(input, nullptr);
    jsize length = env->GetStringLength(input);

    // 创建加密后的字符串
    jchar *encryptedChars = new jchar[length];
    for (jsize i = 0; i < length; i++) {
        jchar c = inputChars[i];

        // 使用内联汇编对每个 Unicode 字符的值加 3，实现加密
        asm volatile (
                "add %0, %1, #3\n"     // 每个字符的 Unicode 值加 3
                : "=r"(c)              // 输出到 c
                : "r"(c)               // 输入 c
                );
        encryptedChars[i] = c;
    }

    // 释放输入字符串的内存
    env->ReleaseStringChars(input, inputChars);
    jstring encryptedString = env->NewString(encryptedChars, length);

    // 释放加密字符串的内存
    delete[] encryptedChars;

    return encryptedString;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_assembly_AssemblyActivity_decryptString(JNIEnv *env, jobject /* this */,
                                                               jstring input) {
    const char *inputStr = env->GetStringUTFChars(input, nullptr);
    std::string decryptedStr(inputStr);

    // 获取输入字符串的 Unicode 码点
    const jchar *inputChars = env->GetStringChars(input, nullptr);
    jsize length = env->GetStringLength(input);

    // 创建解密后的字符串
    jchar *decryptedChars = new jchar[length];
    for (jsize i = 0; i < length; i++) {
        jchar c = inputChars[i];

        // 使用内联汇编对每个 Unicode 字符的值减 3，实现解密
        asm volatile (
                "sub %0, %1, #3\n"     // 每个字符的 Unicode 值减 3
                : "=r"(c)              // 输出到 c
                : "r"(c)               // 输入 c
                );
        decryptedChars[i] = c;
    }

    // 释放输入字符串的内存
    env->ReleaseStringChars(input, inputChars);
    jstring decryptedString = env->NewString(decryptedChars, length);

    // 释放解密字符串的内存
    delete[] decryptedChars;

    return decryptedString;
}



extern "C"
JNIEXPORT jint JNICALL
Java_com_cyrus_example_assembly_AssemblyActivity_addNumbers(JNIEnv *env, jobject, jint a,
                                                            jint b) {
    int result;

#if defined(__aarch64__)
    // ARM64 内联汇编版本
    asm volatile (
            "add %w[result], %w[val1], %w[val2]\n"  // 执行加法
            : [result] "=r" (result)               // 输出操作数
    : [val1] "r" (a), [val2] "r" (b)       // 输入操作数
    );
#elif defined(__arm__)
    // ARM 32-bit 内联汇编版本
    asm volatile (
        "add %[result], %[val1], %[val2]\n"    // 执行加法
        : [result] "=r" (result)               // 输出操作数
        : [val1] "r" (a), [val2] "r" (b)       // 输入操作数
    );
#elif defined(__i386__)
    // x86 内联汇编版本
    asm volatile (
        "addl %[val1], %[val2]\n"
        "movl %[val2], %[result]\n"           // 使用32位 x86 指令完成加法
        : [result] "=r" (result)
        : [val1] "r" (a), [val2] "r" (b)
    );
#elif defined(__x86_64__)
    // x86_64 内联汇编版本
    asm volatile (
        "addl %[val1], %[val2]\n"         // 使用 addl 指令进行 32 位整数加法
        "movl %[val2], %[result]\n"
        : [result] "=r" (result)
        : [val1] "r" (a), [val2] "r" (b)
    );
#else
    // 如果架构不支持，使用 C++ 代码实现
    result = a + b;
#endif

    LOGI("Result of addition: %d", result);
    return result;
}


