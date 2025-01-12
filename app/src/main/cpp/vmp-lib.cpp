#include <jni.h>
#include <string>
#include <sstream>
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
#define MOVE_RESULT_OBJECT_OPCODE 0x0c  // move-result-object 操作码
#define SGET_OBJECT_OPCODE 0x62  // sget-object 操作码
#define INVOKE_VIRTUAL_OPCODE 0x6e  // invoke-virtual 操作码

// 定义支持的寄存器类型（比如 jstring、jboolean、jobject 等等）
using RegisterValue = std::variant<jstring, jboolean, jbyte, jshort, jint, jlong, jfloat, jdouble, jobject>;

// 定义寄存器数量
constexpr size_t NUM_REGISTERS = 10;

// 定义寄存器数组
RegisterValue registers[NUM_REGISTERS];

// 存储不同类型的值到寄存器
template <typename T>
void setRegisterValue(uint8_t reg, T value) {
    // 通过模板将类型 T 存储到寄存器
    registers[reg] = value;
}

// 根据类型从寄存器读取对应的值
jvalue getRegisterAsJValue(int regIdx, const std::string &paramType) {
    const RegisterValue &val = registers[regIdx];
    jvalue result;

    if (paramType == "I") {  // int 类型
        if (std::holds_alternative<jint>(val)) {
            result.i = std::get<jint>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jint.");
        }
    } else if (paramType == "J") {  // long 类型
        if (std::holds_alternative<jlong>(val)) {
            result.j = std::get<jlong>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jlong.");
        }
    } else if (paramType == "F") {  // float 类型
        if (std::holds_alternative<jfloat>(val)) {
            result.f = std::get<jfloat>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jfloat.");
        }
    } else if (paramType == "D") {  // double 类型
        if (std::holds_alternative<jdouble>(val)) {
            result.d = std::get<jdouble>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jdouble.");
        }
    } else if (paramType == "Z") {  // boolean 类型
        if (std::holds_alternative<jboolean>(val)) {
            result.z = std::get<jboolean>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jboolean.");
        }
    } else if (paramType == "B") {  // byte 类型
        if (std::holds_alternative<jbyte>(val)) {
            result.b = std::get<jbyte>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jbyte.");
        }
    } else if (paramType == "S") {  // short 类型
        if (std::holds_alternative<jshort>(val)) {
            result.s = std::get<jshort>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jshort.");
        }
    } else if (paramType == "Ljava/lang/String;") {  // String 类型
        if (std::holds_alternative<jstring>(val)) {
            result.l = std::get<jstring>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jstring.");
        }
    } else if (paramType[0] == 'L') {  // jobject 类型（以 L 开头）
        if (std::holds_alternative<jobject>(val)) {
            result.l = std::get<jobject>(val);
        } else {
            throw std::runtime_error("Type mismatch: Expected jobject.");
        }
    } else {
        throw std::runtime_error("Unsupported parameter type.");
    }

    return result;
}


// 模拟字符串常量池
std::unordered_map <uint32_t, std::string> stringPool = {
        {0x004e00, "input"},
        {0x002c00, "SHA-256"},
        {0x024a00, "getBytes\\(...\\)"},
        {0x034400, "encodeToString\\(...\\)"},
};



// 处理 const-string 指令
void handleConstString(JNIEnv *env, const uint8_t *bytecode, size_t &pc) {
    uint8_t opcode = bytecode[pc];
    if (opcode != CONST_STRING_OPCODE) {  // 检查是否为 const-string 指令
        throw std::runtime_error("Unexpected opcode");
    }

    // 获取目标寄存器索引 reg 和字符串索引
    uint8_t reg = bytecode[pc + 1];  // 目标寄存器
    // 读取字符串索引（第 2、3、4 字节）
    uint32_t stringIndex = (bytecode[pc + 1] << 16) | (bytecode[pc + 2] << 8) | bytecode[pc + 3];

    // 从字符串常量池获取字符串
    const std::string &value = stringPool[stringIndex];

    // 创建 jstring 并将其存储到目标寄存器
    jstring jValue = env->NewStringUTF(value.c_str());
    registers[reg] = jValue;

    // 更新程序计数器
    pc += 4;  // const-string 指令占用 4 字节
}


// 解析方法签名，返回参数类型数组和返回值类型
void parseMethodSignature(const std::string &signature, std::vector<std::string> &paramTypes, std::string &returnType) {
    std::regex paramRegex("\\((.*?)\\)");  // 匹配括号中的参数部分
    std::smatch match;

    if (std::regex_search(signature, match, paramRegex)) {
        // 参数部分在第一个括号内
        std::string params = match[1].str();

        // 解析参数类型
        size_t startPos = 0;
        while (startPos < params.size()) {
            if (params[startPos] == 'L') {
                // 解析类类型（以L开头，类似 Ljava/lang/String;）
                size_t endPos = params.find(';', startPos);
                paramTypes.push_back(params.substr(startPos, endPos - startPos + 1));  // 包括L和;
                startPos = endPos + 1;
            } else {
                // 解析基础类型（如 I, J, F等）
                paramTypes.push_back(std::string(1, params[startPos]));
                startPos++;
            }
        }
    }

    // 返回值类型是签名的括号后部分
    returnType = signature.substr(signature.find(')') + 1);
}


// move-result-object
template <typename T>
void handleMoveResultObject(JNIEnv *env, const uint8_t *bytecode, size_t &pc, T result) {
    uint8_t opcode = bytecode[pc];
    if (opcode == MOVE_RESULT_OBJECT_OPCODE) {
        uint8_t reg = bytecode[pc + 1];  // 目标寄存器
        setRegisterValue(reg, result);
        // 更新程序计数器
        pc += 2;  // move-result-object 指令占用 2 字节
    }
}

// 解析和执行 sget-object 指令
void handleSgetObject(JNIEnv *env, const uint8_t *bytecode, size_t &pc) {
    uint8_t opcode = bytecode[pc];
    if (opcode != SGET_OBJECT_OPCODE) {  // 检查是否为 sget-object
        throw std::runtime_error("Unexpected opcode for sget-object");
    }

    // 解析指令
    uint8_t reg = bytecode[pc + 1];          // 目标寄存器
    uint16_t fieldIndex = (bytecode[pc + 2] << 8) | bytecode[pc + 3]; // 字段索引

    // 类名和方法信息
    std::string className;
    std::string fieldName;
    std::string fieldType;

    // 解析每条指令，依据方法的不同来设置类名、方法名、签名
    switch (fieldIndex) {
        case 0x0900:  // Lkotlin/text/Charsets;->UTF_8:Ljava/nio/charset/Charset;
            className = "kotlin/text/Charsets";
            fieldName = "UTF_8";
            fieldType = "Ljava/nio/charset/Charset;"; // 字段类型为 Charset
            break;
        default:
            throw std::runtime_error("Unknown field index");
    }

    // 1. 获取 Java 类
    jclass clazz = env->FindClass(className.c_str());
    if (clazz == nullptr) {
        LOGI("Failed to find class %s", className.c_str());
        return;
    }

    // 2. 获取静态字段的 Field ID
    jfieldID fieldID = env->GetStaticFieldID(clazz, fieldName.c_str(), fieldType.c_str());
    if (fieldID == nullptr) {
        LOGI("Failed to get field ID for %s", fieldName.c_str());
        return;
    }

    // 3. 获取静态字段的值
    jobject field = env->GetStaticObjectField(clazz, fieldID);
    if (field == nullptr) {
        LOGI("%s field is null", fieldName.c_str());
        return;
    }

    // 保存到目标寄存器
    setRegisterValue(reg, field);

    // 更新程序计数器
    pc += 4; // sget-object 指令占用 4 字节
}


// 解析并执行 invoke-static 指令
void handleInvokeStatic(JNIEnv *env, const uint8_t *bytecode, size_t &pc) {
    uint8_t opcode = bytecode[pc];
    if (opcode != INVOKE_STATIC_OPCODE) {  // 检查是否为 invoke-static
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

    // 根据 methodIndex 来解析并设置类名、方法名、签名
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
    std::vector<std::string> paramTypes;
    std::string returnType;
    parseMethodSignature(methodSignature, paramTypes, returnType);
    int paramCount = paramTypes.size();

    // 动态获取参数
    std::vector <jstring> params(paramCount);
    for (size_t i = 0; i < paramCount; ++i) {
        // 获取寄存器中的值并转化为 JNI 参数
        jvalue value = getRegisterAsJValue(i, paramTypes[i]);
        params[i] = static_cast<jstring>(value.l);
    }

    // 更新程序计数器
    pc += 6;  // invoke-static 指令占用 6 字节

    // 调用静态方法
    // 根据返回值类型决定调用方式
    if (returnType == "V") {  // void 返回值
        if (paramCount > 1) {
            env->CallStaticVoidMethod(targetClass, methodID, params[0], params[1]);
        } else {
            env->CallStaticVoidMethod(targetClass, methodID, params[0]);
        }
    } else if (returnType[0] == 'L') {  // 对象返回值
        jobject result;
        if (paramCount > 1) {
            result = env->CallStaticObjectMethod(targetClass, methodID, params[0], params[1]);
        } else {
            result = env->CallStaticObjectMethod(targetClass, methodID, params[0]);
        }

        // move-result-object
        handleMoveResultObject(env, bytecode, pc, result);

    } else if (returnType == "Z") {  // boolean 返回值
        if (paramCount > 1) {
            jboolean boolResult = env->CallStaticBooleanMethod(targetClass, methodID, params[0],
                                                               params[1]);
            registers[reg1] = boolResult;
        } else {
            jboolean boolResult = env->CallStaticBooleanMethod(targetClass, methodID, params[0]);
            registers[reg1] = boolResult;
        }
    } else if (returnType == "B") {  // byte 返回值
        if (paramCount > 1) {
            jbyte byteResult = env->CallStaticByteMethod(targetClass, methodID, params[0],
                                                         params[1]);
            registers[reg1] = byteResult;
        } else {
            jbyte byteResult = env->CallStaticByteMethod(targetClass, methodID, params[0]);
            registers[reg1] = byteResult;
        }
    } else if (returnType == "S") {  // short 返回值
        if (paramCount > 1) {
            jshort shortResult = env->CallStaticShortMethod(targetClass, methodID, params[0],
                                                            params[1]);
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
            jlong longResult = env->CallStaticLongMethod(targetClass, methodID, params[0],
                                                         params[1]);
            registers[reg1] = longResult;
        } else {
            jlong longResult = env->CallStaticLongMethod(targetClass, methodID, params[0]);
            registers[reg1] = longResult;
        }
    } else if (returnType == "F") {  // float 返回值
        if (paramCount > 1) {
            jfloat floatResult = env->CallStaticFloatMethod(targetClass, methodID, params[0],
                                                            params[1]);
            registers[reg1] = floatResult;
        } else {
            jfloat floatResult = env->CallStaticFloatMethod(targetClass, methodID, params[0]);
            registers[reg1] = floatResult;
        }
    } else if (returnType == "D") {  // double 返回值
        if (paramCount > 1) {
            jdouble doubleResult = env->CallStaticDoubleMethod(targetClass, methodID, params[0],
                                                               params[1]);
            registers[reg1] = doubleResult;
        } else {
            jdouble doubleResult = env->CallStaticDoubleMethod(targetClass, methodID, params[0]);
            registers[reg1] = doubleResult;
        }
    } else {
        throw std::runtime_error("Unsupported return type: " + returnType);
    }
}


// invoke-virtual 指令解析器
void handleInvokeVirtual(JNIEnv* env, const uint8_t* bytecode, size_t& pc) {
    // 解析指令
    uint8_t opcode = bytecode[pc];  // 获取操作码
    if (opcode != INVOKE_VIRTUAL_OPCODE) {  // 确保是 invoke-virtual 操作码
        throw std::runtime_error("Expected invoke-virtual opcode");
    }

    // 获取寄存器数量
    uint8_t regCount = (bytecode[pc + 1] >> 4) & 0xF;

    // 第 5 个字节表示了要使用的寄存器
    uint8_t reg1 = bytecode[pc + 4] & 0xF;         // 低4位表示第一个寄存器
    uint8_t reg2 = (bytecode[pc + 4] >> 4) & 0xF;  // 高4位表示第二个寄存器

    // 读取方法索引（第 2、3、4 字节）
    uint32_t methodIndex = (bytecode[pc + 1] << 16) | (bytecode[pc + 2] << 8) | bytecode[pc + 3];

    // 类名和方法信息
    std::string className;
    std::string methodName;
    std::string methodSignature;

    // 根据 methodIndex 来解析并设置类名、方法名、签名
    switch (methodIndex) {
        case 0x201600:  // Ljava/lang/String;->getBytes(Ljava/nio/charset/Charset;)[B
            className = "java/lang/String";
            methodName = "getBytes";
            methodSignature = "(Ljava/nio/charset/Charset;)[B";
            break;
        case 0x201b00:  // Ljava/security/MessageDigest;->digest([B)[B
            className = "java/security/MessageDigest";
            methodName = "digest";
            methodSignature = "([B)[B";
            break;
        case 0x201d00:  // Ljava/util/Base64$Encoder;->encodeToString([B)Ljava/lang/String;
            className = "java/util/Base64$Encoder";
            methodName = "encodeToString";
            methodSignature = "([B)Ljava/lang/String;";
            break;
        default:
            throw std::runtime_error("Unknown method index: " + std::to_string(methodIndex));
    }

    // 查找类和方法
    jclass clazz = env->FindClass(className.c_str());
    if (!clazz) {
        throw std::runtime_error("Class not found: " + className);
    }

    // 获取方法 ID
    jmethodID methodID = env->GetMethodID(clazz, methodName.c_str(), methodSignature.c_str());
    if (!methodID) {
        throw std::runtime_error("Method not found: " + methodName);
    }

    // 解析方法签名，得到参数个数和返回值类型
    std::vector<std::string> paramTypes;
    std::string returnType;
    parseMethodSignature(methodSignature, paramTypes, returnType);
    int paramCount = paramTypes.size();

    // 目标对象的类型
    std::stringstream ss;
    ss << "L" << className << ";";
    std::string classType = ss.str();

    // 获取目标对象（寄存器中的第一个参数，通常是方法的目标对象）
    jobject targetObject = getRegisterAsJValue(reg1, classType).l;

    // 参数
    std::vector <jvalue> params(paramCount);
    if(paramCount > 0){
        params[0] = getRegisterAsJValue(reg2, paramTypes[0]);
    }

    // 更新程序计数器
    pc += 6;

    // 检查返回值的类型，并调用适当的方法
    if (methodSignature.back() == ')') {  // 如果没有返回值 (void 方法)
        // 调用 void 方法
        env->CallVoidMethodA(targetObject, methodID, params.data());
    } else if (methodSignature.back() == 'B') {  // 如果返回值是 byte 数组
        jbyteArray result = (jbyteArray) env->CallObjectMethodA(targetObject, methodID, params.data());
        // 处理返回的 byte 数组
        if (result) {
            // 可以处理返回的 byte 数组
            handleMoveResultObject(env, bytecode, pc, result);
        }
    } else if (methodSignature.back() == 'L') {  // 如果返回值是对象
        jobject result = env->CallObjectMethodA(targetObject, methodID, params.data());
        // 处理返回的对象
        if (result) {
            // 可以处理返回的对象
            handleMoveResultObject(env, bytecode, pc, result);
        }
    } else if (methodSignature.back() == 'I') {  // 如果返回值是 int
        jint result = env->CallIntMethodA(targetObject, methodID, params.data());
        // 处理返回的 int
        handleMoveResultObject(env, bytecode, pc, result);
    } else if (methodSignature.back() == 'Z') {  // 如果返回值是 boolean
        jboolean result = env->CallBooleanMethodA(targetObject, methodID, params.data());
        // 处理返回的 boolean
        handleMoveResultObject(env, bytecode, pc, result);
    } else if (methodSignature.back() == 'D') {  // 如果返回值是 double
        jdouble result = env->CallDoubleMethodA(targetObject, methodID, params.data());
        // 处理返回的 double
        handleMoveResultObject(env, bytecode, pc, result);
    } else if (methodSignature.back() == 'F') {  // 如果返回值是 float
        jfloat result = env->CallFloatMethodA(targetObject, methodID, params.data());
        // 处理返回的 float
        handleMoveResultObject(env, bytecode, pc, result);
    } else {
        throw std::runtime_error("Unsupported return type in method: " + methodSignature);
    }
}


// Java_com_cyrus_example_vmp_SimpleVMP_execute 实现
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_vmp_SimpleVMP_execute(JNIEnv *env, jobject thiz, jbyteArray bytecodeArray,
                                             jstring input) {

    // 传参存到 v5 寄存器
    registers[5] = input;

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
                    handleConstString(env, bytecode.data(), pc);
                    break;
                case INVOKE_STATIC_OPCODE:
                    handleInvokeStatic(env, bytecode.data(), pc);
                    break;
                case SGET_OBJECT_OPCODE:
                    handleSgetObject(env, bytecode.data(), pc);
                    break;
                case INVOKE_VIRTUAL_OPCODE:
                    handleInvokeVirtual(env, bytecode.data(), pc);
                    break;
                default:
                    throw std::runtime_error("Unknown opcode encountered");
            }
        }

        if (std::holds_alternative<jstring>(registers[0])) {
            return std::get<jstring>(registers[0]);   // 返回寄存器 v0 的值
        }
    } catch (const std::exception &e) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
    }
    return nullptr;
}
