#include <jni.h>
#include <android/log.h>
#include <tomcrypt.h>

#define AES_BLOCKLEN 16 // Block length in bytes AES is 128 b block only
#define AES_KEYLEN 16   // Key length in bytes

#define LOG_TAG "aes_jni.cpp"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


// 将 const char* 转换为字节数组 (IV)
uint8_t *stringToIV(const char *str) {
    auto iv = new uint8_t[AES_BLOCKLEN];
    for (int i = 0; i < AES_BLOCKLEN; ++i) {
        iv[i] = static_cast<uint8_t>(str[i]);
    }
    return iv;
}

// 将 const char* 转换为字节数组 (Key)
uint8_t *stringToSecretKey(const char *str) {
    auto key = new uint8_t[AES_KEYLEN];
    for (int i = 0; i < AES_KEYLEN; ++i) {
        key[i] = static_cast<uint8_t>(str[i]);
    }
    return key;
}

void pkcs5_pad(uint8_t *data, int dataLen, int blockSize, int *paddedLen) {
    int padding = blockSize - (dataLen % blockSize);
    for (int i = 0; i < padding; i++) {
        data[dataLen + i] = (uint8_t)padding;
    }
    *paddedLen = dataLen + padding;
}

int pkcs5_unpad(uint8_t *data, int dataLen) {
    if (dataLen <= 0) return 0;
    uint8_t padding = data[dataLen - 1];
    if (padding > dataLen || padding > 16) return -1; // 不合法的填充
    for (int i = 0; i < padding; i++) {
        if (data[dataLen - 1 - i] != padding) return -1; // 填充不一致
    }
    return dataLen - padding;
}



// AES CBC 加密方法
extern "C" JNIEXPORT jbyteArray JNICALL
__attribute__((annotate("fla"))) Java_com_cyrus_example_aes_NativeAESUtils_aesCBCEncode(JNIEnv *env, jclass clazz, jbyteArray data) {

    uint8_t *dataBytes = (uint8_t *) env->GetByteArrayElements(data, nullptr);
    int dataLen = env->GetArrayLength(data);

    // 转换密钥和 IV 为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");
    uint8_t *ivBytes = stringToIV("CYRUS STUDIO    ");

    // PKCS5Padding
    int paddedLen = dataLen + 16 - (dataLen % 16);
    uint8_t *paddedData = (uint8_t *) malloc(paddedLen);
    memcpy(paddedData, dataBytes, dataLen);
    pkcs5_pad(paddedData, dataLen, 16, &paddedLen);

    // 加密
    int cipher_index = register_cipher(&aes_desc);
    symmetric_CBC cbc;
    if (cbc_start(cipher_index, ivBytes, keyBytes, AES_KEYLEN, 0, &cbc) != CRYPT_OK) {
        LOGD("cbc_start failed.");
        free(paddedData);
        return nullptr;
    }

    uint8_t *output = (uint8_t *) malloc(paddedLen);
    if (cbc_encrypt(paddedData, output, paddedLen, &cbc) != CRYPT_OK) {
        LOGD("cbc_encrypt failed.");
        free(paddedData);
        free(output);
        return nullptr;
    }
    cbc_done(&cbc);

    jbyteArray result = env->NewByteArray(paddedLen);
    env->SetByteArrayRegion(result, 0, paddedLen, reinterpret_cast<jbyte *>(output));

    // 释放资源
    delete[] keyBytes;
    delete[] ivBytes;
    env->ReleaseByteArrayElements(data, (jbyte *) dataBytes, 0);
    free(paddedData);
    free(output);

    return result;
}

// AES CBC 解密方法
extern "C" JNIEXPORT jbyteArray JNICALL
__attribute__((annotate("fla"))) Java_com_cyrus_example_aes_NativeAESUtils_aesCBCDecode(JNIEnv *env, jclass clazz, jbyteArray data) {

    // 获取数据
    uint8_t *dataBytes = (uint8_t *) env->GetByteArrayElements(data, nullptr);
    int dataLen = env->GetArrayLength(data);

    // 注册 AES 算法
    if (register_cipher(&aes_desc) == -1) {
        env->ReleaseByteArrayElements(data, (jbyte *) dataBytes, 0);
        return nullptr;
    }

    // 转换密钥和 IV 为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");
    uint8_t *ivBytes = stringToIV("CYRUS STUDIO    ");

    // 解密
    int cipher_index = register_cipher(&aes_desc);
    symmetric_CBC cbc;
    if (cbc_start(cipher_index, ivBytes, keyBytes, AES_KEYLEN, 0, &cbc) != CRYPT_OK) {
        LOGD("cbc_start failed.");
        return nullptr;
    }

    uint8_t *output = (uint8_t *) malloc(dataLen);
    if (cbc_decrypt(dataBytes, output, dataLen, &cbc) != CRYPT_OK) {
        LOGD("cbc_decrypt failed.");
        free(output);
        return nullptr;
    }
    cbc_done(&cbc);

    // 去除 PKCS5Padding
    int unpaddedLen = pkcs5_unpad(output, dataLen);
    if (unpaddedLen < 0) {
        LOGD("PKCS5 unpadding failed.");
        free(output);
        return nullptr;
    }

    // 返回结果
    jbyteArray result = env->NewByteArray(unpaddedLen);
    env->SetByteArrayRegion(result, 0, unpaddedLen, reinterpret_cast<jbyte *>(output));

    // 释放资源
    delete[] keyBytes;
    delete[] ivBytes;
    free(output);
    env->ReleaseByteArrayElements(data, (jbyte *) dataBytes, 0);

    return result;
}


// AES ECB 加密
extern "C" JNIEXPORT jbyteArray JNICALL
__attribute__((annotate("fla"))) Java_com_cyrus_example_aes_NativeAESUtils_aesECBEncode(
        JNIEnv *env,
        jclass clazz,
        jbyteArray data
) {
    uint8_t *dataBytes = (uint8_t *) env->GetByteArrayElements(data, nullptr);
    int dataLen = env->GetArrayLength(data);

    // PKCS5Padding
    int paddedLen = dataLen + 16 - (dataLen % 16);
    uint8_t *paddedData = (uint8_t *) malloc(paddedLen);
    memcpy(paddedData, dataBytes, dataLen);
    pkcs5_pad(paddedData, dataLen, 16, &paddedLen);

    // 设置 AES ECB
    symmetric_ECB ecb;
    int cipher_index = register_cipher(&aes_desc);

    // 转换密钥为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");

    if (ecb_start(cipher_index, keyBytes, AES_KEYLEN, 0, &ecb) != CRYPT_OK) {
        LOGD("ecb_start failed.");
        delete[] keyBytes;
        free(paddedData);
        return nullptr;
    }

    // 加密
    uint8_t *output = (uint8_t *) malloc(paddedLen);
    if (ecb_encrypt(paddedData, output, paddedLen, &ecb) != CRYPT_OK) {
        LOGD("ecb_encrypt failed.");
        delete[] keyBytes;
        free(paddedData);
        free(output);
        return nullptr;
    }
    ecb_done(&ecb);

    jbyteArray result = env->NewByteArray(paddedLen);
    env->SetByteArrayRegion(result, 0, paddedLen, reinterpret_cast<jbyte *>(output));

    // 释放资源
    delete[] keyBytes;
    free(paddedData);
    free(output);
    env->ReleaseByteArrayElements(data, (jbyte *) dataBytes, 0);

    return result;
}

// AES ECB 解密
extern "C" JNIEXPORT jbyteArray JNICALL
__attribute__((annotate("fla"))) Java_com_cyrus_example_aes_NativeAESUtils_aesECBDecode(
        JNIEnv *env,
        jclass clazz,
        jbyteArray data
) {
    uint8_t *dataBytes = (uint8_t *) env->GetByteArrayElements(data, nullptr);
    int dataLen = env->GetArrayLength(data);

    // 初始化 ECB
    symmetric_ECB ecb;
    int cipher_index = register_cipher(&aes_desc);

    // 转换密钥为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");

    if (ecb_start(cipher_index, keyBytes, AES_KEYLEN, 0, &ecb) != CRYPT_OK) {
        LOGD("ecb_start failed.");
        delete[] keyBytes;
        return nullptr;
    }

    // 解密
    uint8_t *output = (uint8_t *) malloc(dataLen);
    if (ecb_decrypt(dataBytes, output, dataLen, &ecb) != CRYPT_OK) {
        LOGD("ecb_decrypt failed.");
        delete[] keyBytes;
        free(output);
        return nullptr;
    }
    ecb_done(&ecb);

    // 去除 PKCS5Padding
    int unpaddedLen = pkcs5_unpad(output, dataLen);
    if (unpaddedLen < 0) {
        LOGD("PKCS5 unpadding failed.");
        free(output);
        return nullptr;
    }

    jbyteArray result = env->NewByteArray(unpaddedLen);
    env->SetByteArrayRegion(result, 0, unpaddedLen, reinterpret_cast<jbyte *>(output));

    // 释放资源
    delete[] keyBytes;
    free(output);
    env->ReleaseByteArrayElements(data, (jbyte *) dataBytes, 0);

    return result;
}


// AES CTR 加密
extern "C" JNIEXPORT jbyteArray JNICALL
__attribute__((annotate("fla"))) Java_com_cyrus_example_aes_NativeAESUtils_aesCTREncode(
        JNIEnv *env,
        jclass clazz,
        jbyteArray data
) {
    uint8_t *dataBytes = (uint8_t *) env->GetByteArrayElements(data, nullptr);
    int dataLen = env->GetArrayLength(data);

    // 设置 AES CTR
    symmetric_CTR ctr;
    int cipher_index = register_cipher(&aes_desc);

    // 转换密钥和 IV 为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");
    uint8_t *ivBytes = stringToIV("CYRUS STUDIO    ");

    if (ctr_start(cipher_index, ivBytes, keyBytes, AES_KEYLEN, 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr) != CRYPT_OK) {
        LOGD("ctr_start failed.");
        delete[] keyBytes;
        delete[] ivBytes;
        return nullptr;
    }

    // 加密 / 解密
    uint8_t *output = (uint8_t *) malloc(dataLen);
    if (ctr_encrypt(dataBytes, output, dataLen, &ctr) != CRYPT_OK) {
        LOGD("ctr_encrypt failed.");
        delete[] keyBytes;
        delete[] ivBytes;
        free(output);
        return nullptr;
    }
    ctr_done(&ctr);

    jbyteArray result = env->NewByteArray(dataLen);
    env->SetByteArrayRegion(result, 0, dataLen, reinterpret_cast<jbyte *>(output));

    // 释放资源
    delete[] keyBytes;
    delete[] ivBytes;
    free(output);
    env->ReleaseByteArrayElements(data, (jbyte *) dataBytes, 0);
    return result;

}

// AES CTR 解密
extern "C" JNIEXPORT jbyteArray JNICALL
__attribute__((annotate("fla"))) Java_com_cyrus_example_aes_NativeAESUtils_aesCTRDecode(
        JNIEnv *env,
        jclass clazz,
        jbyteArray data
) {
    return Java_com_cyrus_example_aes_NativeAESUtils_aesCTREncode(env, clazz, data);
}

