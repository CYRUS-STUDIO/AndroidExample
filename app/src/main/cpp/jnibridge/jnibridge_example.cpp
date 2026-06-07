#include <jni.h>
#include <string>
#include "jni_bridge.h"
#include <android/log.h>

#define LOG_TAG "jnibridge"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static jobject onCall(JNIEnv *env, void *userData, jclass declaringClass, jobject method, jobjectArray args) {
    std::string methodName = GetMethodName(env, method);
    LOGD("Native onCall invoked for method: %s", methodName.c_str());

    if (methodName == "onCall") {
        jstring msg = (jstring)env->GetObjectArrayElement(args, 0);
        const char *msgChars = env->GetStringUTFChars(msg, nullptr);
        std::string result = "Native received: ";
        result += msgChars;
        env->ReleaseStringUTFChars(msg, msgChars);
        return env->NewStringUTF(result.c_str());
    }

    return nullptr;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_cyrus_example_jnibridge_JNIBridgeActivity_createNativeCallback(JNIEnv *env, jobject thiz) {
    std::vector<std::string> interfaces = {"com/cyrus/example/jnibridge/JNIBridgeActivity$ICallback"};
    return CreateJNIBridgeProxy(env, onCall, nullptr, nullptr, interfaces);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    if (!RegisterJNIBridge(env)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
