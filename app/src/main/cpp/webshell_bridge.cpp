#include <jni.h>
#include <string>
#include <android/log.h>

#define TAG "WebShellBridge"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

extern "C"
JNIEXPORT void JNICALL
Java_com_cyrus_example_webview_WebShellBridge_nativeHandleMessage(
        JNIEnv *env,
        jclass clazz,
        jstring json_) {

    const char *json = env->GetStringUTFChars(json_, 0);

    LOGI("message => %s", json);

    std::string data = json;

    if (data.find("postMessage") != std::string::npos) {

        LOGI("invoke postMessage");

    } else if (data.find("onEvent") != std::string::npos) {

        LOGI("invoke onEvent");

    } else if (data.find("bgBiEvent") != std::string::npos) {

        LOGI("invoke bgBiEvent");

    } else if (data.find("sendEventCustom") != std::string::npos) {

        LOGI("invoke sendEventCustom");
    }

    env->ReleaseStringUTFChars(json_, json);
}