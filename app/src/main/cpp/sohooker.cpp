#include <jni.h>
#include <android/log.h>
#include "shadowhook.h"
#include <link.h>
#include <string.h>

#define TAG "sohooker"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

// 原始函数指针类型
typedef bool (*orig_ca_func_t)(JNIEnv *, jclass, jobject);

static orig_ca_func_t orig_ca_func = nullptr;

// 替换函数
static bool fake_ca(JNIEnv *env, jclass clazz, jobject context) {
    LOGI("🚀 fake_ca called, always return true");
    return true;
}

/**
 * @brief 获取指定共享库（.so 文件）的加载基址。
 *
 * @param libname 要查找的共享库名称的子串，例如 "libtarget.so"。
 *                支持模糊匹配（通过 strstr），无需提供完整路径。
 *
 * @return void* 返回找到的库的加载基地址（dlpi_addr），
 *               如果未找到匹配的库，则返回 nullptr。
 *
 * @example
 *     void* base = get_library_base("libexample.so");
 *     if (base) {
 *         printf("libexample.so 加载基址为: %p\n", base);
 *     }
 */
void *get_library_base(const char *libname) {
    // 定义一个回调数据结构，用于在遍历时传递库名和保存找到的基地址
    struct callback_data {
        const char *libname;
        void *base;
    } data = {libname, nullptr};

    // 使用 dl_iterate_phdr 遍历所有已加载的动态库，传递一个匿名函数（lambda）处理 item
    dl_iterate_phdr([](struct dl_phdr_info *info, size_t, void *data) {

        // 将传入的 data 指针强制转换为 callback_data 类型
        auto *cb = (callback_data *) data;

        // 判断当前库名中是否包含目标库名
        if (info->dlpi_name && strstr(info->dlpi_name, cb->libname)) {
            // 找到匹配库，保存其基地址
            cb->base = (void *) info->dlpi_addr;

            // 返回 1 表示停止迭代（即已经找到了目标库）
            return 1;
        }

        // 继续查找其他库
        return 0;
    }, &data);

    // 返回找到的基址
    return data.base;
}

void print_arch_info() {
#if defined(__aarch64__)
    LOGI("Current architecture: arm64-v8a");
#elif defined(__arm__)
    LOGI("Current architecture: armeabi-v7a");
#elif defined(__i386__)
    LOGI("Current architecture: x86");
#elif defined(__x86_64__)
    LOGI("Current architecture: x86_64");
#else
    LOGI("Unknown architecture");
#endif
}


// 在库加载时自动执行
__attribute__((constructor)) static void init_hook() {
    // 初始化 ShadowHook
    shadowhook_init(SHADOWHOOK_MODE_UNIQUE, true);  // debug 开启日志

    // 获取 so 基址
    void *base = get_library_base("libhexymsb.so");
    if (!base) {
        LOGI("❌ libhexymsb.so not loaded yet");
        return;
    }

    uintptr_t target_addr = 0;

    // 适配不同的架构
#ifdef __aarch64__
    // arm64-v8a
    target_addr = (uintptr_t) base + 0x09C8; // 函数内存地址 = so 基址 + 函数偏移
#else
    // armeabi-v7a
    target_addr = (uintptr_t) base + 0x0610;
#endif

    // 打印当前设备架构信息
    print_arch_info();

    LOGI("🎯 hooking address: %p", (void *) target_addr);

    // Hook
    shadowhook_hook_func_addr(
            (void *) target_addr,             // target
            (void *) fake_ca,                 // replacement
            (void **) &orig_ca_func           // backup
    );
}
