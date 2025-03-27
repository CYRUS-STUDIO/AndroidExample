#include <jni.h>
#include <string>
#include <sstream>  // 用于拼接字符串


// sub 方法：两个整数相减
extern "C" JNIEXPORT jint JNICALL
Java_com_cyrus_example_ollvm_OLLVMActivity_sub(JNIEnv* env, jobject, jint a, jint b) {
    return a - b;
}

// bcf 方法：接收字符串并返回拼接后的字符串
extern "C" JNIEXPORT jstring JNICALL
Java_com_cyrus_example_ollvm_OLLVMActivity_bcf(JNIEnv* env, jobject, jstring input) {
    const char* inputStr = env->GetStringUTFChars(input, nullptr);
    std::string result = std::string("BCF: ") + inputStr;
    env->ReleaseStringUTFChars(input, inputStr);
    return env->NewStringUTF(result.c_str());
}

// fla 方法：两个int相加判断大小并返回结果字符串
extern "C" JNIEXPORT jstring JNICALL
__attribute__((annotate("nobcf,sub"))) Java_com_cyrus_example_ollvm_OLLVMActivity_fla(JNIEnv *env, jobject, jint x, jint y) {
    int sum = x + y;

    // 使用字符串流拼接结果
    std::ostringstream result;

    if (sum < 5) {
        result << "x = " << x << ", y = " << y << ", x + y " << "小于 5";
    } else if(sum == 5){
        result << "x = " << x << ", y = " << y << ", x + y " << "等于 5";
    } else{
        result << "x = " << x << ", y = " << y << ", x + y " << "大于 5";
    }

    // 返回拼接好的字符串
    return env->NewStringUTF(result.str().c_str());
}