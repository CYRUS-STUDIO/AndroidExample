// 引入必要的头文件
#include <jni.h>
#include <string>
#include <fcntl.h>          // 文件控制定义（如 O_RDONLY）
#include <unistd.h>         // 系统调用号（如 __NR_openat）
#include <sys/syscall.h>    // 系统调用函数
#include <android/log.h>

#define LOG_TAG "syscall-lib.cpp"

// 定义 Android 日志宏，用于输出信息级别日志
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 使用 extern "C" 告诉编译器按照 C 语言的方式来编译和链接这个函数
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_syscall_SyscallActivity_readFileWithSyscall(JNIEnv *env, jobject,
                                                                   jstring path) {
    // 将 Java 字符串 (jstring) 转换为 C 字符串 (const char *)
    const char *filePath = env->GetStringUTFChars(path, nullptr);

    // 使用 syscall 系统调用打开文件
    // __NR_openat 是 openat() 系统调用的调用号
    // AT_FDCWD 表示使用当前工作目录
    // O_RDONLY 表示以只读方式打开文件
    int fd = syscall(__NR_openat, AT_FDCWD, filePath, O_RDONLY);

    // 如果文件打开失败，返回错误信息
    if (fd < 0) {
        // 释放通过 GetStringUTFChars 分配的资源
        env->ReleaseStringUTFChars(path, filePath);
        return env->NewStringUTF("Failed to open file");
    }

    // 定义一个缓冲区，用于存储文件内容
    char buffer[1024];

    // 使用 syscall 系统调用读取文件内容
    // __NR_read 是 read() 系统调用的调用号
    // 读取的内容存储到 buffer 中，最多读取 sizeof(buffer) - 1 字节
    ssize_t bytesRead = syscall(__NR_read, fd, buffer, sizeof(buffer) - 1);

    // 如果读取失败，返回错误信息
    if (bytesRead < 0) {
        // 关闭文件描述符
        syscall(__NR_close, fd);

        // 释放通过 GetStringUTFChars 分配的资源
        env->ReleaseStringUTFChars(path, filePath);
        return env->NewStringUTF("Failed to read file");
    }

    // 使用 syscall 系统调用关闭文件
    syscall(__NR_close, fd);

    // 释放通过 GetStringUTFChars 分配的资源
    env->ReleaseStringUTFChars(path, filePath);

    // 确保缓冲区以 '\0' 结尾（C 字符串需要以 '\0' 作为结束符）
    buffer[bytesRead] = '\0';

    // 输出读取到的文件内容到控制台
    LOGI("File content: %s\n", buffer);

    // 将读取到的文件内容转换为 Java 字符串 (jstring) 并返回
    return env->NewStringUTF(buffer);
}


//__attribute__((naked)) long raw_syscall(long __number, ...) {
//    __asm__ __volatile__(
//            "MOV             R12, SP\n"
//            "PUSH            {R4-R7}\n"
//            "MOV             R7, R0\n"
//            "MOV             R0, R1\n"
//            "MOV             R1, R2\n"
//            "MOV             R2, R3\n"
//            "LDM             R12, {R3-R6}\n"
//            "SVC             0\n"
//            "POP             {R4-R7}\n"
//            "mov             pc, lr");
//}

extern "C" long raw_syscall(long __number, ...);

// 读取文件内容
std::string read_file(const char *filePath) {
    char buffer[1024] = {0};

    // 调用 raw_syscall 打开文件
    int fd = raw_syscall(SYS_openat, 0, filePath, O_RDONLY, 0);
    if (fd < 0) {
        return "Failed to open file";
    }

    // 调用 raw_syscall 读取文件
    ssize_t bytesRead = raw_syscall(SYS_read, fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        raw_syscall(SYS_close, fd);
        return "Failed to read file";
    }

    // 关闭文件
    raw_syscall(SYS_close, fd);

    // 输出读取到的文件内容到控制台
    LOGI("File content: %s\n", buffer);

    return std::string(buffer);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_syscall_SyscallActivity_readFileWithAssemblySyscall(JNIEnv *env, jobject,
                                                                           jstring path) {
    // 将 Java 字符串 (jstring) 转换为 C 字符串 (const char *)
    const char *filePath = env->GetStringUTFChars(path, nullptr);

    std::string file_content = read_file(filePath);

    // 释放通过 GetStringUTFChars 分配的资源
    env->ReleaseStringUTFChars(path, filePath);

    return env->NewStringUTF(file_content.c_str());
}
