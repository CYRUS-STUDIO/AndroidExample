#include <jni.h>
#include <string>

extern "C" JNIEXPORT jint JNICALL
Java_com_cyrus_example_jniexample_JNIExampleActivity_intExample(JNIEnv *env, jobject, jint value) {
    return value * 2; // 简单的操作示例
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_cyrus_example_jniexample_JNIExampleActivity_doubleExample(JNIEnv *env, jobject,
                                                                   jdouble value) {
    return value + 1.0;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_cyrus_example_jniexample_JNIExampleActivity_booleanExample(JNIEnv *env, jobject,
                                                                    jboolean value) {
    return !value;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_jniexample_JNIExampleActivity_stringExample(JNIEnv *env, jobject,
                                                                   jstring value) {
    const char *str = env->GetStringUTFChars(value, 0);
    std::string result = "JNI Received: ";
    result += str;
    env->ReleaseStringUTFChars(value, str);
    return env->NewStringUTF(result.c_str());
}
