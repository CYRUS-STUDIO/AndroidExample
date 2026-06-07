#include <jni.h>
#include <string>
#include "openssl_utils/ossl_http_client.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_openssl_OpenSSLActivity_fetchHttpData(JNIEnv *env, jobject thiz, jstring url_jstr) {
    const char *url = env->GetStringUTFChars(url_jstr, nullptr);
    std::string result = ossl_utils::fetch_http_data(url, 10);
    env->ReleaseStringUTFChars(url_jstr, url);
    return env->NewStringUTF(result.c_str());
}
