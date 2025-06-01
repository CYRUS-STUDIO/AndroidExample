#include <jni.h>
#include <unistd.h>
#include <string>
#include <android/log.h>
#include <sstream>
#include <signal.h>
#include <set>
#include <vector>
#include <fstream>
#include <dlfcn.h>
#include <regex>

#define LOG_TAG "AntiFART"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cyrus_example_fart_AntiFART_detectFARTByReflection(JNIEnv *env, jclass clazz) {
    const char *targetClass = "android/app/ActivityThread";
    jclass target = env->FindClass(targetClass);
    if (target == nullptr) {
        LOGE("Failed to find ActivityThread");
        return JNI_FALSE;
    }

    struct {
        const char *name;
        const char *sig;
    } suspiciousMethods[] = {
            {"getClassField",       "(Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/reflect/Field;"},
            {"getClassFieldObject", "(Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/Object;Ljava/lang/String;)Ljava/lang/Object;"},
            {"invokeStaticMethod",  "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Class;[Ljava/lang/Object;)Ljava/lang/Object;"},
            {"getFieldOjbect",      "(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/String;)Ljava/lang/Object;"},
            {"getClassloader",      "()Ljava/lang/ClassLoader;"},
            {"loadClassAndInvoke",  "(Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/reflect/Method;)V"},
            {"fart",                "()V"},
            {"fartwithClassloader", "(Ljava/lang/ClassLoader;)V"},
            {"fartthread",          "()V"},
    };

    for (const auto &method: suspiciousMethods) {
        jmethodID mid = env->GetStaticMethodID(target, method.name, method.sig);
        if (env->ExceptionCheck()) {
            env->ExceptionClear(); // 避免崩溃
            continue;
        }
        if (mid != nullptr) {
            LOGE("Detected FART method: %s", method.name);
            kill(getpid(), SIGKILL);
            return JNI_TRUE;
        }
    }

    return JNI_FALSE;
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrus_example_fart_AntiFART_dumpMethods(JNIEnv *env, jclass, jstring className) {
    const char *clsName = env->GetStringUTFChars(className, nullptr);

    // 替换 . 为 /，Java 类名用点，JNI 中要用斜杠
    std::string classPath(clsName);
    std::replace(classPath.begin(), classPath.end(), '.', '/');

    jclass clazz = env->FindClass(classPath.c_str());
    env->ReleaseStringUTFChars(className, clsName);

    if (clazz == nullptr) {
        return env->NewStringUTF("Class not found");
    }

    jmethodID getDeclaredMethods = env->GetMethodID(
            env->FindClass("java/lang/Class"),
            "getDeclaredMethods",
            "()[Ljava/lang/reflect/Method;"
    );
    if (getDeclaredMethods == nullptr) {
        return env->NewStringUTF("getDeclaredMethods not found");
    }

    jobjectArray methodArray = (jobjectArray) env->CallObjectMethod(clazz, getDeclaredMethods);
    if (methodArray == nullptr) {
        return env->NewStringUTF("No methods");
    }

    jsize len = env->GetArrayLength(methodArray);
    jclass methodClass = env->FindClass("java/lang/reflect/Method");
    jmethodID toStringMid = env->GetMethodID(methodClass, "toString", "()Ljava/lang/String;");

    std::ostringstream oss;

    for (jsize i = 0; i < len; ++i) {
        jobject methodObj = env->GetObjectArrayElement(methodArray, i);
        jstring methodStr = (jstring) env->CallObjectMethod(methodObj, toStringMid);
        const char *cstr = env->GetStringUTFChars(methodStr, nullptr);

        oss << cstr << "\n";

        env->ReleaseStringUTFChars(methodStr, cstr);
        env->DeleteLocalRef(methodStr);
        env->DeleteLocalRef(methodObj);
    }

    std::string result = oss.str();
    return env->NewStringUTF(result.c_str());
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_cyrus_example_fart_AntiFART_listLoadedFiles(JNIEnv *env, jclass) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    std::vector<std::string> paths;

    while (std::getline(maps, line)) {
        std::size_t pathPos = line.find('/');
        if (pathPos != std::string::npos) {
            std::string path = line.substr(pathPos);
            if (std::find(paths.begin(), paths.end(), path) == paths.end()) {
                paths.push_back(path);
            }
        }
    }

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray result = env->NewObjectArray(paths.size(), stringClass, nullptr);
    for (size_t i = 0; i < paths.size(); ++i) {
        env->SetObjectArrayElement(result, i, env->NewStringUTF(paths[i].c_str()));
    }

    return result;
}


void *lookup_symbol(const char *libraryname, const char *symbolname) {
    void *imagehandle = dlopen(libraryname, RTLD_GLOBAL | RTLD_NOW);
    if (imagehandle != nullptr) {
        void *sym = dlsym(imagehandle, symbolname);
        if (sym != nullptr) {
            return sym;
        }
    }
    return nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cyrus_example_fart_AntiFART_hasSymbolInSO(JNIEnv *env, jclass, jstring soName, jstring symbolName) {
    const char *so = env->GetStringUTFChars(soName, nullptr);
    const char *symbol = env->GetStringUTFChars(symbolName, nullptr);

    void *sym = lookup_symbol(so, symbol);
    jboolean result = sym != nullptr ? JNI_TRUE : JNI_FALSE;

    LOGI("lookup_symbol: so=%s, symbol=%s, result=%s", so, symbol, result == JNI_TRUE ? "FOUND" : "NOT FOUND");

    env->ReleaseStringUTFChars(soName, so);
    env->ReleaseStringUTFChars(symbolName, symbol);

    return result;
}

// so 黑名单函数特征
std::vector<std::string> so_symbols_blacklist = {
        "dumpDexFileByExecute",
        "dumpArtMethod",
        "myfartInvoke",
        "DexFile_dumpMethodCode"
};

// 读取 /proc/self/maps 获取加载的 .so 路径
std::set<std::string> get_loaded_so_paths() {
    std::set<std::string> so_paths;
    std::ifstream maps("/proc/self/maps");
    std::string line;
    std::regex so_regex(".+\\.so(\\s|$)");

    while (std::getline(maps, line)) {
        std::size_t path_pos = line.find('/');
        if (path_pos != std::string::npos) {
            std::string path = line.substr(path_pos);
            if (std::regex_search(path, so_regex)) {
                so_paths.insert(path);
            }
        }
    }
    return so_paths;
}

// 读取文件内容为字符串
std::string read_file_content(const std::string &path) {
    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        LOGI("Failed to open: %s", path.c_str());
        return "";
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    std::string buffer(size, 0);
    fread(&buffer[0], 1, size, file);
    fclose(file);

    return buffer;
}

// 单词边界检查
bool is_word_boundary(char ch) {
    return !std::isalnum(static_cast<unsigned char>(ch)) && ch != '_';
}

// 返回匹配到的特征列表
std::vector<std::string> get_matched_signatures(const std::string &content, const std::vector<std::string> &patterns) {
    std::vector<std::string> matched;
    for (const auto &pattern : patterns) {
        size_t pos = content.find(pattern);
        if (pos != std::string::npos) {
            // 类似 DexFile_dumpMethodCode 这种，带 _ 的不需要做边界检查
            if (pattern.find('_') != std::string::npos) {
                matched.push_back(pattern);
            }else{
                // 单词边界检查
                // 这样就不会匹配 farther、himmelfart，但可以匹配像 void fart()、"fart"、 call fart 等形式。
                char prev = (pos == 0) ? '\0' : content[pos - 1];
                char next = (pos + pattern.length() < content.size()) ? content[pos + pattern.length()] : '\0';

                if (is_word_boundary(prev) && is_word_boundary(next)) {
                    matched.push_back(pattern);
                }
            }
        }
    }
    return matched;
}


// JNI 方法：检测已加载 .so 中是否包含黑名单符号
extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_cyrus_example_fart_AntiFART_detectFartInLoadedSO(JNIEnv *env, jclass clazz) {
    std::vector<std::string> detected_logs;
    auto so_paths = get_loaded_so_paths();

    for (const auto &path: so_paths) {
        std::string content = read_file_content(path);
        if (!content.empty()) {
            std::vector<std::string> matched = get_matched_signatures(content, so_symbols_blacklist);
            if (!matched.empty()) {
                std::ostringstream oss;
                oss << "[FART DETECTED] " << path << " => ";
                for (size_t i = 0; i < matched.size(); ++i) {
                    oss << matched[i];
                    if (i != matched.size() - 1) oss << ", ";
                }
                LOGI("%s", oss.str().c_str());
                detected_logs.push_back(oss.str());
            }
        }
    }

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray result = env->NewObjectArray(detected_logs.size(), stringClass, nullptr);
    for (int i = 0; i < detected_logs.size(); ++i) {
        env->SetObjectArrayElement(result, i, env->NewStringUTF(detected_logs[i].c_str()));
    }

    return result;
}


// dex 黑名单函数特征
const std::vector<std::string> dex_method_blacklist = {
        "loadClassAndInvoke",
        "fart",
        "fartwithClassloader",
        "fartthread"
};


// 读取 /proc/self/maps 获取加载的 dex 或 dex 相关文件路径
std::set<std::string> get_loaded_dex_paths() {
    std::set<std::string> dex_paths;
    std::ifstream maps("/proc/self/maps");
    std::string line;

    // 匹配 dex、odex、vdex、art、apk、jar 文件
    std::regex dex_regex(R"((\.dex|\.odex|\.vdex|\.art|\.apk|\.jar)(\s|$))");

    while (std::getline(maps, line)) {
        std::size_t path_pos = line.find('/');
        if (path_pos != std::string::npos) {
            std::string path = line.substr(path_pos);
            if (std::regex_search(path, dex_regex)) {
                dex_paths.insert(path);
            }
        }
    }
    return dex_paths;
}


// JNI 方法：检测已加载 dex 中是否包含黑名单符号
extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_cyrus_example_fart_AntiFART_detectFartInLoadedDex(JNIEnv *env, jclass clazz) {
    std::vector<std::string> detected_logs;
    auto dex_paths = get_loaded_dex_paths();

    for (const auto &path: dex_paths) {
        std::string content = read_file_content(path);
        if (!content.empty()) {
            std::vector<std::string> matched = get_matched_signatures(content, dex_method_blacklist);
            if (!matched.empty()) {
                std::ostringstream oss;
                oss << "[FART DETECTED] " << path << " => ";
                for (size_t i = 0; i < matched.size(); ++i) {
                    oss << matched[i];
                    if (i != matched.size() - 1) oss << ", ";
                }
                LOGI("%s", oss.str().c_str());
                detected_logs.push_back(oss.str());
            }
        }
    }

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray result = env->NewObjectArray(detected_logs.size(), stringClass, nullptr);
    for (int i = 0; i < detected_logs.size(); ++i) {
        env->SetObjectArrayElement(result, i, env->NewStringUTF(detected_logs[i].c_str()));
    }

    return result;
}
