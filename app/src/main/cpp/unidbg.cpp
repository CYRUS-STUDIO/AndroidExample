#include <jni.h>
#include <android/log.h>
#include <cstring>

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