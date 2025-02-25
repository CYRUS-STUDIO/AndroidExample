
#include <jni.h>
#include <string>
#include <cstring>
#include <android/log.h>

// Android Log 标签
#define LOG_TAG "UnicornJNI"


// 实现加法运算
extern "C"
JNIEXPORT jint JNICALL
Java_com_cyrus_example_unicorn_UnicornActivity_add(JNIEnv *env, jobject thiz, jint a, jint b) {
    return a + b;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_unicorn_UnicornActivity_simulateJni(JNIEnv* env, jobject /* this */) {
    // 模拟通过 NewStringUTF 创建一个新的 jstring
    const char* utf_string = "Hello from JNI!";
    jstring newString = env->NewStringUTF(utf_string);

    // 通过 GetStringUTFChars 获取 UTF-8 字符串
    const char* utf_chars = env->GetStringUTFChars(newString, nullptr);

    // 打印 GetStringUTFChars 获取的字符串
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "GetStringUTFChars 返回的字符串: %s", utf_chars);

    // 将 std::string 转换为 const char* 以便传递给 NewStringUTF
    std::string result_str = "Simulated JNI result: " + std::string(utf_chars);
    jstring result = env->NewStringUTF(result_str.c_str());

    // 释放 UTF 字符串
    env->ReleaseStringUTFChars(newString, utf_chars);

    return result;
}


extern "C" JNIEXPORT void JNICALL
Java_com_cyrus_example_unicorn_UnicornActivity_emptyFunction(JNIEnv *env, jobject thiz) {
// 空的 JNI 函数实现
// 目前没有任何操作
}
