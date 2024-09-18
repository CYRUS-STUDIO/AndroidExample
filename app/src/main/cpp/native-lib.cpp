#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_MainActivity_getNativeString(JNIEnv *env, jobject /* this */) {
    std::string hello = "Hello From Native";
    return env->NewStringUTF(hello.c_str());
}
