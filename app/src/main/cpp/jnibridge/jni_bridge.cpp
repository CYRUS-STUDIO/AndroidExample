#include "jni_bridge.h"
#include <android/log.h>

#define LOG_TAG "jni_bridge"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace {

    class CallbackBridgeHandler final : public JNIBridgeInterface {
    public:
        CallbackBridgeHandler(JNIBridgeInvokeCallback invokeCallback, void *userData, JNIBridgeDeleteCallback deleteCallback)
                : invokeCallback_(invokeCallback),
                  userData_(userData),
                  deleteCallback_(deleteCallback) {
        }

        ~CallbackBridgeHandler() override {
            if (deleteCallback_ != nullptr) {
                deleteCallback_(userData_);
            }
        }

        jobject Invoke(JNIEnv *env, jclass declaringClass, jobject method, jobjectArray args) override {
            if (invokeCallback_ == nullptr) {
                return nullptr;
            }
            return invokeCallback_(env, userData_, declaringClass, method, args);
        }

    private:
        JNIBridgeInvokeCallback invokeCallback_;
        void *userData_;
        JNIBridgeDeleteCallback deleteCallback_;
    };

    jobject native_invoke(JNIEnv *env, jclass, jlong ptr, jclass declaringClass, jobject method, jobjectArray args) {
        auto *handler = reinterpret_cast<JNIBridgeInterface *>(ptr);
        if (handler == nullptr) {
            return nullptr;
        }
        return handler->Invoke(env, declaringClass, method, args);
    }

    void native_delete(JNIEnv *env, jclass, jlong ptr) {
        auto *handler = reinterpret_cast<JNIBridgeInterface *>(ptr);
        if (handler != nullptr) {
            handler->Release(env);
        }
        delete handler;
    }

    bool CheckException(JNIEnv *env, const char *step) {
        if (!env->ExceptionCheck()) {
            return false;
        }
        LOGD("JNIBridge exception at %s", step);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

} // namespace

jobject CreateJNIBridgeProxy(JNIEnv *env, std::unique_ptr<JNIBridgeInterface> handler, const std::vector<std::string> &interfaceClassNames) {
    if (env == nullptr || handler == nullptr || interfaceClassNames.empty()) {
        return nullptr;
    }

    jclass bridgeClass = env->FindClass("com/cyrus/example/jnibridge/JNIBridge");
    if (bridgeClass == nullptr || CheckException(env, "FindClass JNIBridge")) {
        return nullptr;
    }

    jclass classClass = env->FindClass("java/lang/Class");
    if (classClass == nullptr || CheckException(env, "FindClass Class")) {
        env->DeleteLocalRef(bridgeClass);
        return nullptr;
    }

    jobjectArray interfaces = env->NewObjectArray(static_cast<jsize>(interfaceClassNames.size()), classClass, nullptr);
    if (interfaces == nullptr || CheckException(env, "NewObjectArray interfaces")) {
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(bridgeClass);
        return nullptr;
    }

    for (jsize i = 0; i < static_cast<jsize>(interfaceClassNames.size()); ++i) {
        jclass interfaceClass = env->FindClass(interfaceClassNames[i].c_str());
        if (interfaceClass == nullptr || CheckException(env, "FindClass interface")) {
            env->DeleteLocalRef(interfaces);
            env->DeleteLocalRef(classClass);
            env->DeleteLocalRef(bridgeClass);
            return nullptr;
        }
        env->SetObjectArrayElement(interfaces, i, interfaceClass);
        env->DeleteLocalRef(interfaceClass);
        if (CheckException(env, "SetObjectArrayElement interface")) {
            env->DeleteLocalRef(interfaces);
            env->DeleteLocalRef(classClass);
            env->DeleteLocalRef(bridgeClass);
            return nullptr;
        }
    }

    jmethodID newProxyMethod = env->GetStaticMethodID(bridgeClass, "newInterfaceProxy", "(J[Ljava/lang/Class;)Ljava/lang/Object;");
    if (newProxyMethod == nullptr || CheckException(env, "GetStaticMethodID newInterfaceProxy")) {
        env->DeleteLocalRef(interfaces);
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(bridgeClass);
        return nullptr;
    }

    jlong ptr = reinterpret_cast<jlong>(handler.get());
    jobject proxy = env->CallStaticObjectMethod(bridgeClass, newProxyMethod, ptr, interfaces);
    if (!CheckException(env, "CallStaticObjectMethod newInterfaceProxy") && proxy != nullptr) {
        handler.release();
    }

    env->DeleteLocalRef(interfaces);
    env->DeleteLocalRef(classClass);
    env->DeleteLocalRef(bridgeClass);
    return proxy;
}

jobject CreateJNIBridgeProxy(JNIEnv *env,
                             JNIBridgeInvokeCallback invokeCallback,
                             void *userData,
                             JNIBridgeDeleteCallback deleteCallback,
                             const std::vector<std::string> &interfaceClassNames) {
    return CreateJNIBridgeProxy(env, std::make_unique<CallbackBridgeHandler>(invokeCallback, userData, deleteCallback), interfaceClassNames);
}

void DisableJNIBridgeProxy(JNIEnv *env, jobject proxy) {
    if (env == nullptr || proxy == nullptr) {
        return;
    }

    jclass bridgeClass = env->FindClass("com/cyrus/example/jnibridge/JNIBridge");
    if (bridgeClass == nullptr || CheckException(env, "FindClass JNIBridge disable")) {
        return;
    }

    jmethodID disableMethod = env->GetStaticMethodID(bridgeClass, "disableInterfaceProxy", "(Ljava/lang/Object;)V");
    if (disableMethod != nullptr && !CheckException(env, "GetStaticMethodID disableInterfaceProxy")) {
        env->CallStaticVoidMethod(bridgeClass, disableMethod, proxy);
        CheckException(env, "CallStaticVoidMethod disableInterfaceProxy");
    }

    env->DeleteLocalRef(bridgeClass);
}

bool RegisterJNIBridge(JNIEnv *env) {
    jclass clazz = env->FindClass("com/cyrus/example/jnibridge/JNIBridge");
    if (clazz == nullptr) {
        LOGD("FindClass JNIBridge failed");
        return false;
    }

    JNINativeMethod methods[] = {
            {
                    "invoke",
                    "(JLjava/lang/Class;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;",
                    reinterpret_cast<void *>(native_invoke)
            },
            {
                    "delete",
                    "(J)V",
                    reinterpret_cast<void *>(native_delete)
            }
    };

    bool ok = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) == JNI_OK;
    if (!ok) {
        LOGD("Register JNIBridge failed");
    }

    env->DeleteLocalRef(clazz);
    return ok;
}


std::string JStringToString(JNIEnv *env, jstring value) {
    if (value == nullptr) {
        return "";
    }
    const char *chars = env->GetStringUTFChars(value, nullptr);
    if (chars == nullptr) {
        return "";
    }
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    return result;
}

std::string GetMethodName(JNIEnv *env, jobject method) {
    if (method == nullptr) {
        return "";
    }

    jclass methodClass = env->FindClass("java/lang/reflect/Method");
    jmethodID getNameMethod = methodClass == nullptr ? nullptr : env->GetMethodID(methodClass, "getName", "()Ljava/lang/String;");
    auto name = getNameMethod == nullptr ? nullptr : static_cast<jstring>(env->CallObjectMethod(method, getNameMethod));

    std::string result = JStringToString(env, name);

    if (name != nullptr) {
        env->DeleteLocalRef(name);
    }
    if (methodClass != nullptr) {
        env->DeleteLocalRef(methodClass);
    }
    return result;
}
