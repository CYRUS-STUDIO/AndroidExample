#ifndef JNI_BRIDGE_H
#define JNI_BRIDGE_H

#include <jni.h>
#include <memory>
#include <string>
#include <vector>

class JNIBridgeInterface {
public:
    virtual ~JNIBridgeInterface() = default;

    virtual jobject Invoke(JNIEnv *env, jclass declaringClass, jobject method, jobjectArray args) = 0;

    virtual void Release(JNIEnv *env) {
    }
};

using JNIBridgeInvokeCallback = jobject (*)(JNIEnv *env,
                                            void *userData,
                                            jclass declaringClass,
                                            jobject method,
                                            jobjectArray args);

using JNIBridgeDeleteCallback = void (*)(void *userData);

jobject CreateJNIBridgeProxy(JNIEnv *env,
                             std::unique_ptr<JNIBridgeInterface> handler,
                             const std::vector<std::string> &interfaceClassNames);

jobject CreateJNIBridgeProxy(JNIEnv *env,
                             JNIBridgeInvokeCallback invokeCallback,
                             void *userData,
                             JNIBridgeDeleteCallback deleteCallback,
                             const std::vector<std::string> &interfaceClassNames);

void DisableJNIBridgeProxy(JNIEnv *env, jobject proxy);

bool RegisterJNIBridge(JNIEnv *env);

std::string GetMethodName(JNIEnv *env, jobject method);

#endif
