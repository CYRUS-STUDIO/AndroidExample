#include <unistd.h>
#include <android/log.h>
#include <jni.h>
#include <string>
#include <stddef.h>
#include "shadowhook.h"

#include "dex/dex_file.h"
#include "dex/art_method.h"
#include "dex/class_accessor.h"

using namespace cyurs;

#define LOG_TAG "cyrus_studio_hook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


int g_sdkLevel = 0;

__attribute__ ((constructor)) void init() {
    // api level
    g_sdkLevel = android_get_device_api_level();
}

// 原始 execve 函数指针
int (*orig_execve)(const char *__file, char *const *__argv, char *const *__envp);

// 替代 execve 实现
int my_execve(const char *__file, char *const *__argv, char *const *__envp) {

    LOGI("execve called: %s", __file);

    if (__file && strstr(__file, "dex2oat")) {
        LOGW("Blocked dex2oat execution: %s", __file);
        // 返回失败，模拟 dex2oat 调用失败
        return -1;
    }

    // 调用原始 execve
    return orig_execve(__file, __argv, __envp);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cyrus_example_hook_CyrusStudioHook_hookExecve(JNIEnv *, jclass) {
    void *handle = shadowhook_hook_sym_name(
            "libc.so",  // 函数所在模块
            "execve", // 要 hook 的符号名
            reinterpret_cast<void *>(my_execve),
            reinterpret_cast<void **>(&orig_execve)
    );

    if (handle != nullptr) {
        LOGI("Successfully hooked execve");
    } else {
        LOGW("Failed to hook execve");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern char **environ;

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cyrus_example_hook_CyrusStudioHook_dex2oat(JNIEnv *env, jclass,
                                                    jstring dexPath_,
                                                    jstring oatPath_) {
    const char *dexPath = env->GetStringUTFChars(dexPath_, nullptr);
    const char *oatPath = env->GetStringUTFChars(oatPath_, nullptr);

    const char *dex2oatPath = "/system/bin/dex2oat";

    char dexArg[256];
    char oatArg[256];
    char isaArg[] = "--instruction-set=arm64";
    snprintf(dexArg, sizeof(dexArg), "--dex-file=%s", dexPath);
    snprintf(oatArg, sizeof(oatArg), "--oat-file=%s", oatPath);

    const char *argv[] = {
            dex2oatPath,
            dexArg,
            oatArg,
            isaArg,
            "--runtime-arg", "-Xbootclasspath:/apex/com.android.art/javalib/core-oj.jar:/apex/com.android.art/javalib/core-libart.jar",
            "--boot-image=/apex/com.android.art/javalib/boot.art",
            nullptr
    };

    LOGI("Calling dex2oat with: %s %s %s", dexArg, oatArg, isaArg);

    int ret = execve(dex2oatPath, (char *const *) argv, environ);

    LOGE("execve called with ret: %s", ret);

    env->ReleaseStringUTFChars(dexPath_, dexPath);
    env->ReleaseStringUTFChars(oatPath_, oatPath);

    return false; // execve 不返回，除非失败
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *(*orig_LoadMethod)(void *, void *, void *, void *, void *);

void *my_LoadMethod(void *linker, void *dex_file, void *method, void *klass_handle, void *dst) {

    // DexFile
    std::string location;
    uint8_t *begin = nullptr;
    uint64_t dexSize = 0;
    if (g_sdkLevel >= 35) {
        auto *dexFileV35 = (V35::DexFile *) dex_file;
        location = dexFileV35->location_;
        begin = (uint8_t *) dexFileV35->begin_;
        dexSize = dexFileV35->header_->file_size_;
    } else if (g_sdkLevel >= __ANDROID_API_P__) {
        auto *dexFileV28 = (V28::DexFile *) dex_file;
        location = dexFileV28->location_;
        begin = (uint8_t *) dexFileV28->begin_;
        dexSize = dexFileV28->size_ == 0 ? dexFileV28->header_->file_size_ : dexFileV28->size_;
    } else {
        auto *dexFileV21 = (V21::DexFile *) dex_file;
        location = dexFileV21->location_;
        begin = (uint8_t *) dexFileV21->begin_;
        dexSize = dexFileV21->size_ == 0 ? dexFileV21->header_->file_size_ : dexFileV21->size_;
    }

    // 打印 DexFile 信息
    LOGI("[pid=%d][API=%d] enter my_LoadMethod:\n  DexFile Base    = %p\n  DexFile Size    = %zu bytes\n  DexFile Location= %s",
         getpid(), g_sdkLevel, begin, dexSize, location.c_str());

    // 调用原始函数，使 ArtMethod 数据填充完成
    void *result = orig_LoadMethod(linker, dex_file, method, klass_handle, dst);

    // ArtMethod
    uint32_t dex_code_item_offset_ = -1;
    uint32_t dex_method_index_;
    if (g_sdkLevel >= 31) {
        auto *dstV31 = (V31::ArtMethod *) dst;
        auto classAccessor_method = reinterpret_cast<Method &>(method);
        dex_code_item_offset_ = classAccessor_method.code_off_;
        dex_method_index_ = dstV31->dex_method_index_;
    } else {
        auto *dstV28 = (V28::ArtMethod *) dst;
        dex_code_item_offset_ = dstV28->dex_code_item_offset_;
        dex_method_index_ = dstV28->dex_method_index_;
    }

    // 打印 Method 信息
    LOGI("[pid=%d][API=%d] enter my_LoadMethod:\n"
         "  ArtMethod.dex_code_item_offset_ = 0x%x\n"
         "  ArtMethod.dex_method_index_     = %d",
         getpid(), g_sdkLevel, dex_code_item_offset_, dex_method_index_);

    return result;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_cyrus_example_hook_CyrusStudioHook_hookLoadMethod(JNIEnv *, jclass) {
    void *handle = shadowhook_hook_sym_name(
            "libart.so",
            "_ZN3art11ClassLinker10LoadMethodERKNS_7DexFileERKNS_13ClassAccessor6MethodENS_6HandleINS_6mirror5ClassEEEPNS_9ArtMethodE", // 要 hook 的符号名
            reinterpret_cast<void *>(my_LoadMethod),
            reinterpret_cast<void **>(&orig_LoadMethod)
    );

    if (handle != nullptr) {
        LOGI("Successfully hooked LoadMethod");
    } else {
        LOGW("Failed to hook LoadMethod");
    }
}
