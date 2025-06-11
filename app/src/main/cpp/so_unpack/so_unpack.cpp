#include <jni.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>
#include <elf.h>
#include <link.h>
#include <dlfcn.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <sys/stat.h>
#include "soinfo.h"

#define LOG_TAG "SoUnpack"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C"
JNIEXPORT jint JNICALL
Java_com_cyrus_example_so_1unpack_MemoryProtector_protect(JNIEnv *env, jobject thiz,
                                                        jlong address, jlong size,
                                                        jint prot) {
    long pageSize = sysconf(_SC_PAGESIZE);
    long pageStart = address & ~(pageSize - 1);
    long pageEnd = (address + size + pageSize - 1) & ~(pageSize - 1);
    size_t alignedSize = pageEnd - pageStart;

    int result = mprotect(reinterpret_cast<void *>(pageStart), alignedSize, prot);
    if (result != 0) {
        LOGE("mprotect failed at %p size %zu prot %d", (void *)pageStart, alignedSize, prot);
    } else {
        LOGI("mprotect success at %p size %zu prot %d", (void *)pageStart, alignedSize, prot);
    }

    return result;
}


struct ModuleInfo {
    uintptr_t base = 0;
    size_t size = 0;
    std::string path;
    std::string permissions;
};

// 获取模块信息
ModuleInfo get_module_info(const char* module_name) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return {};

    char line[512];
    uintptr_t start = 0, end = 0;
    char perms[5] = {0};
    char path[256] = {0};
    ModuleInfo info;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, module_name)) {
            // 格式: start-end perms offset dev inode pathname
            if (sscanf(line, "%lx-%lx %4s %*s %*s %*s %255[^\n]", &start, &end, perms, path) >= 3) {
                info.base = start;
                info.size = end - start;
                info.permissions = perms;
                info.path = path;
                break;  // 只获取第一个匹配到的模块
            }
        }
    }

    fclose(fp);
    return info;
}


// 获取模块基址（通过 /proc/self/maps 或 dlopen + dladdr 等）
uintptr_t get_module_base(const char* module_name) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, module_name)) {
            sscanf(line, "%lx-%*lx %*s %*s %*s %*d", &base);
            break;
        }
    }
    fclose(fp);
    return base;
}

// 解析 linker.so 的 ELF，找到 __dl__ZL6solist 地址
soinfo* find_solist() {
    const char* lib_name = "linker64";
    ModuleInfo linkerInfo = get_module_info(lib_name);
    if (linkerInfo.base == 0) {
        LOGI("Failed to find base of %s", lib_name);
        return nullptr;
    }

    LOGI("linker64 module info:\n  base: 0x%lx\n  size: 0x%lx\n  path: %s\n  permissions: %s",
         linkerInfo.base, linkerInfo.size,
         linkerInfo.path.empty() ? "N/A" : linkerInfo.path.c_str(),
         linkerInfo.permissions.c_str());

    int fd = open(linkerInfo.path.c_str(), O_RDONLY);
    if (fd < 0) {
        LOGI("Failed to open %s", linkerInfo.path.c_str());
        return nullptr;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size <= 0) {
        close(fd);
        return nullptr;
    }
    lseek(fd, 0, SEEK_SET);

    void* file_map = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (file_map == MAP_FAILED) return nullptr;

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)file_map;
    Elf64_Shdr* shdrs = (Elf64_Shdr*)((char*)file_map + ehdr->e_shoff);
    const char* shstrtab = (const char*)file_map + shdrs[ehdr->e_shstrndx].sh_offset;

    Elf64_Shdr* symtab_shdr = nullptr;
    Elf64_Shdr* strtab_shdr = nullptr;

    // 查找 SHT_SYMTAB 和对应的字符串表
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab_shdr = &shdrs[i];
        }
        if (shdrs[i].sh_type == SHT_STRTAB && strcmp(shstrtab + shdrs[i].sh_name, ".strtab") == 0) {
            strtab_shdr = &shdrs[i];
        }
    }

    if (!symtab_shdr || !strtab_shdr) {
        LOGI("Failed to locate SYMTAB or STRTAB");
        munmap(file_map, file_size);
        return nullptr;
    }

    Elf64_Sym* symtab = (Elf64_Sym*)((char*)file_map + symtab_shdr->sh_offset);
    const char* strtab = (const char*)file_map + strtab_shdr->sh_offset;
    int symcount = symtab_shdr->sh_size / sizeof(Elf64_Sym);

    const char* target_name = "__dl__ZL6solist";
    soinfo* solist = nullptr;

    for (int i = 0; i < symcount; i++) {
        const char* name = strtab + symtab[i].st_name;
        LOGI("Found: %s", name);
        if (strcmp(name, target_name) == 0) {
            uintptr_t offset = symtab[i].st_value;
            solist = (soinfo*)(linkerInfo.base + offset);
            LOGI("Found solist: %s at offset 0x%lx -> address 0x%lx", name, offset, (uintptr_t)solist);
            break;
        }
    }

    munmap(file_map, file_size);
    return solist;
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_cyrus_example_so_1unpack_SoInfoUtils_enumerateSoModules(JNIEnv* env, jobject thiz) {
    soinfo* current = find_solist();
    if (!current) {
        LOGI("solist not found");
        return nullptr;
    }

    // 统计数量
    int count = 0;
    soinfo* tmp = current;
    while (tmp != nullptr) {
        const char* name = tmp->soname_;
        if (name != nullptr && strlen(name) > 0) {
            count++;
        }
        tmp = tmp->next;
    }

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray result = env->NewObjectArray(count, stringClass, nullptr);

    // 填充结果
    int idx = 0;
    while (current != nullptr) {
        const char* name = current->soname_;
        if (name != nullptr && strlen(name) > 0) {
            jstring jname = env->NewStringUTF(name);
            env->SetObjectArrayElement(result, idx++, jname);
            env->DeleteLocalRef(jname);
        }
        current = current->next;
    }

    return result;
}
