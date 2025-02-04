#include <jni.h>

// 实现加法运算
extern "C"
JNIEXPORT jint JNICALL
Java_com_cyrus_example_unicorn_UnicornActivity_add(JNIEnv *env, jobject thiz, jint a, jint b) {
    return a + b;
}
