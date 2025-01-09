#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "vmp-lib.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


//1a004e00            | const-string v0, "input"
//712020000500        | invoke-static{v5, v0}, Lkotlin/jvm/internal/Intrinsics;->checkNotNullParameter(Ljava/lang/Object;Ljava/lang/String;)V
//1a002c00            | const-string v0, "SHA-256"
//71101c000000        | invoke-static{v0}, Ljava/security/MessageDigest;->getInstance(Ljava/lang/String;)Ljava/security/MessageDigest;
//0c00                | move-result-object v0
//62010900            | sget-object v1, Lkotlin/text/Charsets;->UTF_8:Ljava/nio/charset/Charset;
//6e2016001500        | invoke-virtual{v5, v1}, Ljava/lang/String;->getBytes(Ljava/nio/charset/Charset;)[B
//0c01                | move-result-object v1
//1a024a00            | const-string v2, "getBytes\(...\)"
//71201f002100        | invoke-static{v1, v2}, Lkotlin/jvm/internal/Intrinsics;->checkNotNullExpressionValue(Ljava/lang/Object;Ljava/lang/String;)V
//6e201b001000        | invoke-virtual{v0, v1}, Ljava/security/MessageDigest;->digest([B)[B
//0c01                | move-result-object v1
//71001e000000        | invoke-static{}, Ljava/util/Base64;->getEncoder()Ljava/util/Base64$Encoder;
//0c02                | move-result-object v2
//6e201d001200        | invoke-virtual{v2, v1}, Ljava/util/Base64$Encoder;->encodeToString([B)Ljava/lang/String;
//0c02                | move-result-object v2
//1a034400            | const-string v3, "encodeToString\(...\)"
//71201f003200        | invoke-static{v2, v3}, Lkotlin/jvm/internal/Intrinsics;->checkNotNullExpressionValue(Ljava/lang/Object;Ljava/lang/String;)V
//1102                | return-object v2


#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <regex>

#define CONST_STRING_OPCODE 0x1A  // const-string 操作码
#define INVOKE_STATIC_OPCODE 0x71  // invoke-static 操作码

// 定义支持的寄存器类型（比如 string、jobject、jboolean 等等）
using RegisterValue = std::variant<std::string, jobject, jboolean , jbyte, jshort, jint, jlong, jfloat, jdouble>;

std::unordered_map<uint8_t, RegisterValue> registers;  // 寄存器，支持多种类型


jclass g_JFloatClass = nullptr;
jmethodID g_JFloatConstructor = nullptr;
jclass g_JDoubleClass = nullptr;
jmethodID g_JDoubleConstructor = nullptr;

// 初始化缓存类和构造函数 ID
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // 缓存 Float 和 Double 类及其构造函数
    g_JFloatClass = env->FindClass("java/lang/Float");
    if (g_JFloatClass == nullptr) return JNI_ERR;
    g_JFloatConstructor = env->GetMethodID(g_JFloatClass, "<init>", "(F)V");

    g_JDoubleClass = env->FindClass("java/lang/Double");
    if (g_JDoubleClass == nullptr) return JNI_ERR;
    g_JDoubleConstructor = env->GetMethodID(g_JDoubleClass, "<init>", "(D)V");

    return JNI_VERSION_1_6;
}


jobject getRegisterAsJNIParam(JNIEnv* env, uint8_t reg) {
    const RegisterValue& val = registers[reg];

    if (std::holds_alternative<std::string>(val)) {
        // 如果是 std::string，返回 JNI 字符串
        return env->NewStringUTF(std::get<std::string>(val).c_str());
    }
    else if (std::holds_alternative<jobject>(val)) {
        // 如果是 jobject，直接返回
        return std::get<jobject>(val);
    }
    else if (std::holds_alternative<jboolean>(val)) {
        // 如果是 jboolean，直接返回
        return reinterpret_cast<jobject>(std::get<jboolean>(val));  // 将 jboolean 转为 jobject
    }
    else if (std::holds_alternative<jbyte>(val)) {
        // 如果是 jbyte，返回 jbyte 类型的值
        return reinterpret_cast<jobject>(std::get<jbyte>(val));  // 将 jbyte 转为 jobject
    }
    else if (std::holds_alternative<jshort>(val)) {
        // 如果是 jshort，返回 jshort 类型的值
        return reinterpret_cast<jobject>(std::get<jshort>(val));  // 将 jshort 转为 jobject
    }
    else if (std::holds_alternative<jint>(val)) {
        // 如果是 jint，返回 jint 类型的值
        return reinterpret_cast<jobject>(std::get<jint>(val));  // 将 jint 转为 jobject
    }
    else if (std::holds_alternative<jlong>(val)) {
        // 如果是 jlong，返回 jlong 类型的值
        return reinterpret_cast<jobject>(std::get<jlong>(val));  // 将 jlong 转为 jobject
    }
    else if (std::holds_alternative<jfloat>(val)) {
        // 如果是 jfloat，返回 jfloat 类型的值，不能转换为 jobject
        return env->NewObject(g_JFloatClass, g_JFloatConstructor, std::get<jfloat>(val));
    }
    else if (std::holds_alternative<jdouble>(val)) {
        // 如果是 jdouble，返回 jdouble 类型的值，不能转换为 jobject
        return env->NewObject(g_JDoubleClass, g_JDoubleConstructor, std::get<jdouble>(val));
    }

    // 如果没有匹配的类型，则返回 nullptr
    return nullptr;
}


// 模拟字符串常量池
std::unordered_map <uint32_t, std::string> stringPool = {
        {0x004E, "input"}
};

// 处理 const-string 指令
void handleConstString(const uint8_t *bytecode, size_t &pc) {
    uint8_t opcode = bytecode[pc];
    if (opcode != CONST_STRING_OPCODE) {  // 检查是否为 const-string 指令
        throw std::runtime_error("Unexpected opcode");
    }

    // 获取目标寄存器索引 reg 和字符串索引
    uint8_t reg = bytecode[pc + 1];  // 目标寄存器
    uint16_t stringIndex = *(uint16_t * ) & bytecode[pc + 2];  // 字符串索引

    // 从字符串常量池获取字符串
    const std::string &value = stringPool[stringIndex];

    // 将字符串存储到目标寄存器
    registers[reg] = value;

    // 更新程序计数器
    pc += 4;  // const-string 指令占用 4 字节
}

// 解析方法签名，返回参数个数和返回值类型
void parseMethodSignature(const std::string& signature, size_t& paramCount, std::string& returnType) {
    std::regex paramRegex("\\((.*)\\)");  // 匹配括号中的参数
    std::smatch match;

    if (std::regex_search(signature, match, paramRegex)) {
        // 参数部分在第一个括号内
        std::string params = match[1].str();
        paramCount = std::count(params.begin(), params.end(), ';');  // 计算参数个数
    }

    // 返回值类型是签名的括号后部分
    returnType = signature.substr(signature.find(')') + 1);
}

// 解析并执行 invoke-static 指令
void handleInvokeStatic(JNIEnv* env, const uint8_t* bytecode, size_t& pc) {
    uint8_t opcode = bytecode[pc];
    if (opcode != 0x71) {  // 检查是否为 invoke-static
        throw std::runtime_error("Unexpected opcode for invoke-static");
    }

    // 第 5 个字节表示了要使用的寄存器
    uint8_t reg1 = bytecode[pc + 4] & 0xF;         // 低4位表示第一个寄存器
    uint8_t reg2 = (bytecode[pc + 4] >> 4) & 0xF;  // 高4位表示第二个寄存器

    // 读取方法索引（第 2、3、4 字节）
    uint32_t methodIndex = (bytecode[pc + 1] << 16) | (bytecode[pc + 2] << 8) | bytecode[pc + 3];

    // 类名和方法信息
    std::string className;
    std::string methodName;
    std::string methodSignature;

    // 解析每条指令，依据方法的不同来设置类名、方法名、签名
    switch (methodIndex) {
        case 0x202000:  // checkNotNullParameter
            className = "kotlin/jvm/internal/Intrinsics";
            methodName = "checkNotNullParameter";
            methodSignature = "(Ljava/lang/Object;Ljava/lang/String;)V";
            break;
        case 0x101c00:  // getInstance (MessageDigest)
            className = "java/security/MessageDigest";
            methodName = "getInstance";
            methodSignature = "(Ljava/lang/String;)Ljava/security/MessageDigest;";
            break;
        case 0x201f00:  // checkNotNullExpressionValue
            className = "kotlin/jvm/internal/Intrinsics";
            methodName = "checkNotNullExpressionValue";
            methodSignature = "(Ljava/lang/Object;Ljava/lang/String;)V";
            break;
        case 0x001e00:  // getEncoder (Base64)
            className = "java/util/Base64";
            methodName = "getEncoder";
            methodSignature = "()Ljava/util/Base64$Encoder;";
            break;
        default:
            throw std::runtime_error("Unknown method index");
    }

    // 获取目标类
    jclass targetClass = env->FindClass(className.c_str());
    if (targetClass == nullptr) {
        throw std::runtime_error("Class not found: " + className);
    }

    // 获取方法 ID
    jmethodID methodID = env->GetStaticMethodID(targetClass, methodName.c_str(), methodSignature.c_str());
    if (methodID == nullptr) {
        throw std::runtime_error("Method not found: " + methodName);
    }

    // 解析方法签名，得到参数个数和返回值类型
    size_t paramCount = 0;
    std::string returnType;
    parseMethodSignature(methodSignature, paramCount, returnType);

    // 动态获取参数
    std::vector<jobject> params(paramCount);
    for (size_t i = 0; i < paramCount; ++i) {
        // 获取寄存器中的值并转化为 JNI 参数
        params[i] = getRegisterAsJNIParam(env, (i == 0) ? reg1 : reg2);  // 这里只处理了 reg1 和 reg2，实际情况可能更复杂
    }

    // 调用静态方法
    // 根据返回值类型决定调用方式
    if (returnType == "V") {  // void 返回值
        if (paramCount > 1) {
            env->CallStaticVoidMethod(targetClass, methodID, params[0], params[1]);
        } else {
            env->CallStaticVoidMethod(targetClass, methodID, params[0]);
        }
    } else if (returnType == "L") {  // 对象返回值
        if (paramCount > 1) {
            jobject result = env->CallStaticObjectMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = result;  // 将结果存储到寄存器
        } else {
            jobject result = env->CallStaticObjectMethod(targetClass, methodID, params[0]);
            registers[reg1] = result;  // 将结果存储到寄存器
        }
    } else if (returnType == "Z") {  // boolean 返回值
        if (paramCount > 1) {
            jboolean boolResult = env->CallStaticBooleanMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = boolResult;
        } else {
            jboolean boolResult = env->CallStaticBooleanMethod(targetClass, methodID, params[0]);
            registers[reg1] = boolResult;
        }
    } else if (returnType == "B") {  // byte 返回值
        if (paramCount > 1) {
            jbyte byteResult = env->CallStaticByteMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = byteResult;
        } else {
            jbyte byteResult = env->CallStaticByteMethod(targetClass, methodID, params[0]);
            registers[reg1] = byteResult;
        }
    } else if (returnType == "S") {  // short 返回值
        if (paramCount > 1) {
            jshort shortResult = env->CallStaticShortMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = shortResult;
        } else {
            jshort shortResult = env->CallStaticShortMethod(targetClass, methodID, params[0]);
            registers[reg1] = shortResult;
        }
    } else if (returnType == "I") {  // int 返回值
        if (paramCount > 1) {
            jint intResult = env->CallStaticIntMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = intResult;
        } else {
            jint intResult = env->CallStaticIntMethod(targetClass, methodID, params[0]);
            registers[reg1] = intResult;
        }
    } else if (returnType == "J") {  // long 返回值
        if (paramCount > 1) {
            jlong longResult = env->CallStaticLongMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = longResult;
        } else {
            jlong longResult = env->CallStaticLongMethod(targetClass, methodID, params[0]);
            registers[reg1] = longResult;
        }
    } else if (returnType == "F") {  // float 返回值
        if (paramCount > 1) {
            jfloat floatResult = env->CallStaticFloatMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = floatResult;
        } else {
            jfloat floatResult = env->CallStaticFloatMethod(targetClass, methodID, params[0]);
            registers[reg1] = floatResult;
        }
    } else if (returnType == "D") {  // double 返回值
        if (paramCount > 1) {
            jdouble doubleResult = env->CallStaticDoubleMethod(targetClass, methodID, params[0], params[1]);
            registers[reg1] = doubleResult;
        } else {
            jdouble doubleResult = env->CallStaticDoubleMethod(targetClass, methodID, params[0]);
            registers[reg1] = doubleResult;
        }
    } else {
        throw std::runtime_error("Unsupported return type: " + returnType);
    }

    // 异常检查
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        throw std::runtime_error("Error while invoking static method: " + methodName);
    }

    // 更新程序计数器
    pc += 6;  // invoke-static 指令占用 6 字节
}


// Java_com_cyrus_example_vmp_SimpleVMP_execute 实现
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_vmp_SimpleVMP_execute(JNIEnv *env, jobject thiz, jbyteArray bytecodeArray, jstring input) {

    // 获取 jstring 的 UTF-8 字符数组
    const char* cstr = env->GetStringUTFChars(input, nullptr);

    // 将 char* 转换为 std::string
    std::string input_str(cstr);

    // 释放获取的字符数组
    env->ReleaseStringUTFChars(input, cstr);

    // 传参存到 v5 寄存器
    registers[5] = input_str;

    // 获取字节码数组的长度
    jsize length = env->GetArrayLength(bytecodeArray);
    std::vector <uint8_t> bytecode(length);
    env->GetByteArrayRegion(bytecodeArray, 0, length, reinterpret_cast<jbyte *>(bytecode.data()));

    size_t pc = 0;  // 程序计数器
    try {
        // 执行字节码中的指令
        while (pc < bytecode.size()) {
            uint8_t opcode = bytecode[pc];

            switch (opcode) {
                case CONST_STRING_OPCODE:
                    handleConstString(bytecode.data(), pc);
                    break;
                case INVOKE_STATIC_OPCODE:
                    handleInvokeStatic(env, bytecode.data(), pc);
                    break;
                default:
                    throw std::runtime_error("Unknown opcode encountered");
            }
        }

        return env->NewStringUTF(std::get<std::string>(registers[0]).c_str());  // 返回寄存器 v0 的值
    } catch (const std::exception &e) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
        return nullptr;
    }
}
