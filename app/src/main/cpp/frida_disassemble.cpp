#include <jni.h>
#include <android/log.h>

#define LOG_TAG "FridaDisassemble"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// 实现 add 方法：计算 1 + 1 + 1
jint native_add(JNIEnv *env, jobject obj) {
    return 1 + 1 + 1;
}

// 定义 JNI 方法映射
static const JNINativeMethod methods[] = {
        {"add", "()I", (void *)native_add} // 方法名, 方法签名, C++函数指针
};

// 动态注册 JNI 方法
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // 获取 Java 类
    jclass clazz = env->FindClass("com/cyrus/example/frida/disassemble/FridaDisassembleActivity");
    if (clazz == nullptr) {
        LOGD("[-] Failed to find class FridaDisassembleActivity");
        return JNI_ERR;
    }

    // 注册 native 方法
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        LOGD("[-] Failed to register native methods");
        return JNI_ERR;
    }

    LOGD("[+] Successfully registered native methods");
    return JNI_VERSION_1_6;
}
