#include <jni.h>
#include <android/log.h>
#include <cstring>
#include <string>

// Android Log 标签
#define LOG_TAG "Unidbg"

// 定义 init 函数
extern "C" void _init(void) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Shared library initialized (init).");
}

// 使用 __attribute__((constructor)) 标记函数为构造函数，库加载时自动调用
__attribute__((constructor))
void init_array1() {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Shared library initialized (init_array1).");
}

__attribute__((constructor))
void init_array2() {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Shared library initialized (init_array2).");
}


// 计算6个整数的和
extern "C" int add(int a, int b, int c, int d, int e, int f) {
    int sum = a + b + c + d + e + f;
    // 使用 Log 打印计算的和
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Sum of integers: %d", sum);
    return sum;
}

// 打印字符串和字符串长度，最后返回字符串长度
extern "C" int string_length(const char* str) {
    int length = strlen(str);
    // 使用 Log 打印字符串和长度
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "String: %s", str);
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Length: %d", length);

    // 返回字符串长度
    return length;
}


// native add 函数
extern "C" JNIEXPORT jint JNICALL native_add(
        JNIEnv* env, jobject thiz, jint a, jint b, jint c, jint d, jint e, jint f) {
    // 计算并返回和
    return a + b + c + d + e + f;
}

// 动态注册 add 方法
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // 获取 UnidbgActivity 类
    jclass clazz = env->FindClass("com/cyrus/example/unidbg/UnidbgActivity");
    if (clazz == nullptr) {
        return JNI_ERR;
    }

    // 动态注册 add 方法
    JNINativeMethod methods[] = {
            {"add", "(IIIIII)I", (void*)native_add}
    };

    // 注册方法
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}


// 实现静态的 native 方法 staticAdd
extern "C" JNIEXPORT jint JNICALL Java_com_cyrus_example_unidbg_UnidbgActivity_staticAdd(
        JNIEnv* env, jclass clazz, jint a, jint b, jint c, jint d, jint e, jint f) {
    // 计算并返回和
    return a + b + c + d + e + f;
}

// 静态注册 stringLength 方法
extern "C" JNIEXPORT jint JNICALL Java_com_cyrus_example_unidbg_UnidbgActivity_stringLength(
        JNIEnv* env, jobject thiz, jstring str) {
    const char* str_chars = env->GetStringUTFChars(str, nullptr);
    int length = strlen(str_chars);
    env->ReleaseStringUTFChars(str, str_chars);
    return length;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_unidbg_UnidbgActivity_sign(JNIEnv *env, jobject thiz, jstring content) {
    // 获取 content 字符串
    const char *contentChars = env->GetStringUTFChars(content, nullptr);

    // 获取静态变量 a 和 非静态变量 b
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID aField = env->GetStaticFieldID(clazz, "a", "Ljava/lang/String;");
    jfieldID bField = env->GetFieldID(clazz, "b", "Ljava/lang/String;");

    jstring a = (jstring) env->GetStaticObjectField(clazz, aField);
    jstring b = (jstring) env->GetObjectField(thiz, bField);

    // 将 a, content 和 b 拼接
    const char *aChars = env->GetStringUTFChars(a, nullptr);
    const char *bChars = env->GetStringUTFChars(b, nullptr);

    std::string combined = std::string(aChars) + contentChars + std::string(bChars);

    // 释放字符串
    env->ReleaseStringUTFChars(content, contentChars);
    env->ReleaseStringUTFChars(a, aChars);
    env->ReleaseStringUTFChars(b, bChars);

    // 调用 base64 方法
    jmethodID base64Method = env->GetMethodID(clazz, "base64", "(Ljava/lang/String;)Ljava/lang/String;");
    jstring combinedStr = env->NewStringUTF(combined.c_str());
    jstring base64Result = (jstring) env->CallObjectMethod(thiz, base64Method, combinedStr);

    return base64Result;
}
