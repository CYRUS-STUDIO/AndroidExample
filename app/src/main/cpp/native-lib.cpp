#include <jni.h>
#include <string>
#include <iostream>


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



extern "C"
JNIEXPORT jobject JNICALL
Java_com_cyrus_example_jniexample_JNIExample_arrayExample(JNIEnv *env, jclass clazz, jobjectArray array) {
    // 获取数组长度
    jsize length = env->GetArrayLength(array);
    if (length < 3) {
        return env->NewStringUTF("Array size is too small");
    }

    // 获取第一个元素
    jobject firstElement = env->GetObjectArrayElement(array, 0);
    if (firstElement == nullptr) {
        return env->NewStringUTF("First element is null");
    }

    // 获取第一个元素的整数值
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID intValueMethod = env->GetMethodID(integerClass, "intValue", "()I");
    jint firstValue = env->CallIntMethod(firstElement, intValueMethod);

    // 获取第三个元素并转换为字符串
    jobject thirdElement = env->GetObjectArrayElement(array, 2);
    if (thirdElement == nullptr) {
        return env->NewStringUTF("Third element is null");
    }

    jclass stringClass = env->FindClass("java/lang/String");
    if (!env->IsInstanceOf(thirdElement, stringClass)) {
        return env->NewStringUTF("Third element is not a string");
    }

    const char *thirdString = env->GetStringUTFChars((jstring) thirdElement, nullptr);

    if (firstValue == 283) {
        // 异或操作
        std::string result;
        for (int i = 0; thirdString[i] != '\0'; i++) {
            result += thirdString[i] ^ 0x5A; // 使用 0x5A 进行异或
        }
        env->ReleaseStringUTFChars((jstring) thirdElement, thirdString);
        return env->NewStringUTF(result.c_str());
    } else if (firstValue == 282) {
        // 返回字符串长度
        env->ReleaseStringUTFChars((jstring) thirdElement, thirdString);
        return env->NewObject(integerClass, env->GetMethodID(integerClass, "<init>", "(I)V"), (jint) strlen(thirdString));
    }

    env->ReleaseStringUTFChars((jstring) thirdElement, thirdString);
    return env->NewStringUTF("Unsupported value");
}
