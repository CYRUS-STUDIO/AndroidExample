#include <jni.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "anti_debug.c"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 标志变量，判断 SIGTRAP 是否被捕获
volatile int sigtrap_caught = 0;

// SIGTRAP 信号处理函数
void sigtrap_handler(int sig) {
    LOGI("Caught SIGTRAP. No debugger present.");
    sigtrap_caught = 1; // 标记 SIGTRAP 被捕获
}

// JNI 方法，触发 SIGTRAP 信号并检测调试器
JNIEXPORT jboolean JNICALL
Java_com_cyrus_example_antidebug_AntiDebug_detectDebugger(JNIEnv *env, jobject instance) {
    // 注册 SIGTRAP 处理器
    signal(SIGTRAP, sigtrap_handler);

    // 触发 SIGTRAP 信号
    raise(SIGTRAP);

    // 检查信号是否被捕获
    if (sigtrap_caught) {
        LOGI("No debugger detected.");
        return JNI_FALSE; // 没有检测到调试器
    } else {
        // 如果信号未被捕获，说明有调试器
        LOGI("Debugger detected! The program will exit in 3 seconds...");
        sleep(3); // 等待 3 秒
        exit(EXIT_FAILURE); // 退出程序
        return JNI_TRUE; // 返回 true，表示检测到调试器
    }
}
