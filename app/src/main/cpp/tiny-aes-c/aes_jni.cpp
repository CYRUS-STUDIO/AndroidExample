#include <jni.h>
#include <string>
#include <stdint.h>
#include <fstream>
#include "aes.h"

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

jbyteArray clientByteArrayFromUnit8(JNIEnv *env, uint8_t *array, int length) {
    jbyteArray output = env->NewByteArray(length);
    env->SetByteArrayRegion(output, 0, length, reinterpret_cast<jbyte *>(array));
    return output;
}

static uint8_t *encodeByteArray(
        uint8_t *message,
        uint32_t length,
        uint8_t *key,
        uint8_t *initialization_vector,
        int mode
) {
    struct AES_ctx ctx{};
    if (initialization_vector == nullptr) {
        AES_init_ctx(&ctx, key);
    } else {
        AES_init_ctx_iv(&ctx, key, initialization_vector);
    }
    uint32_t last_loop_size = length % AES_BLOCKLEN;
    uint32_t iterations = length / AES_BLOCKLEN;
    if (length % AES_BLOCKLEN != 0) {
        iterations++;
    }
    size_t buffer_size = AES_BLOCKLEN * sizeof(uint8_t);
    uint8_t buffer[buffer_size];
    auto *output = (uint8_t *) calloc(iterations * AES_BLOCKLEN, sizeof(uint8_t));
    for (uint32_t iterationIndex = 0; iterationIndex < iterations; iterationIndex++) {
        memset(buffer, 0, buffer_size);
        if (iterationIndex == iterations - 1 && last_loop_size > 0) {
            memcpy(
                    buffer,
                    message + iterationIndex * AES_BLOCKLEN * sizeof(uint8_t),
                    last_loop_size
            );
        } else {
            memcpy(
                    buffer,
                    message + iterationIndex * AES_BLOCKLEN * sizeof(uint8_t),
                    buffer_size
            );
        }

        switch (mode) {
            case 1: // CBC
                AES_CBC_encrypt_buffer(&ctx, buffer, buffer_size);
                break;
            case 2: // ECB
                AES_ECB_encrypt(&ctx, buffer);
                break;
            case 3: // CTR
                AES_CTR_xcrypt_buffer(&ctx, buffer, buffer_size);
                break;
            default:
                AES_ECB_encrypt(&ctx, buffer);
                break;
        }

        memcpy(
                output + iterationIndex * AES_BLOCKLEN * sizeof(uint8_t),
                buffer,
                buffer_size
        );
    }
    return output;
}


static uint8_t *decodeByteArray(
        uint8_t *message,
        uint32_t length,
        uint8_t *key,
        uint8_t *initialization_vector,
        int mode
) {
    struct AES_ctx ctx{};
    if (initialization_vector == nullptr) {
        AES_init_ctx(&ctx, key);
    } else {
        AES_init_ctx_iv(&ctx, key, initialization_vector);
    }
    uint32_t iterations = length / AES_BLOCKLEN;
    if (length % AES_BLOCKLEN != 0) {
        iterations++;
    }
    size_t buffer_size = AES_BLOCKLEN * sizeof(uint8_t);
    uint8_t buffer[buffer_size];
    auto *output = (uint8_t *) calloc(iterations * AES_BLOCKLEN, sizeof(uint8_t));
    for (uint32_t iterationIndex = 0; iterationIndex < iterations; iterationIndex++) {
        memset(&buffer, 0, buffer_size);
        memcpy(
                buffer,
                message + iterationIndex * AES_BLOCKLEN * sizeof(uint8_t),
                buffer_size
        );

        switch (mode) {
            case 1: // CBC
                AES_CBC_decrypt_buffer(&ctx, buffer, buffer_size);
                break;
            case 2: // ECB
                AES_ECB_decrypt(&ctx, buffer);
                break;
            case 3: // CTR
                AES_CTR_xcrypt_buffer(&ctx, buffer, buffer_size);
                break;
            default:
                AES_ECB_decrypt(&ctx, buffer);
                break;
        }

        memcpy(
                output + iterationIndex * AES_BLOCKLEN * sizeof(uint8_t),
                buffer,
                buffer_size
        );
    }
    return output;
}


// AES 加密方法
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_cyrus_example_aes_NativeAESUtils_aesEncode(JNIEnv *env, jclass clazz, jbyteArray data, jint mode) {

    auto *message_to_encode = (uint8_t *) env->GetByteArrayElements(data, 0);

    // 转换密钥和 IV 为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");
    uint8_t *ivBytes = stringToIV("CYRUS STUDIO    ");


    auto message_to_encode_length = env->GetArrayLength(data);
    auto *output = encodeByteArray(
            message_to_encode,
            static_cast<uint32_t>(message_to_encode_length),
            keyBytes,
            ivBytes,
            mode
    );

    env->ReleaseByteArrayElements(data, (jbyte *) message_to_encode, 0);

    // 释放资源
    delete[] keyBytes;
    delete[] ivBytes;

    auto iterations = message_to_encode_length / AES_BLOCKLEN;
    if (message_to_encode_length % AES_BLOCKLEN != 0) {
        iterations++;
    }
    return clientByteArrayFromUnit8(env, output, iterations * AES_BLOCKLEN * sizeof(uint8_t));
}

// AES 解密方法
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_cyrus_example_aes_NativeAESUtils_aesDecode(JNIEnv *env, jclass clazz, jbyteArray data, jint mode) {

    auto *message_to_decode = (uint8_t *) env->GetByteArrayElements(data, 0);

    // 转换密钥和 IV 为字节数组
    uint8_t *keyBytes = stringToSecretKey("CYRUS STUDIO    ");
    uint8_t *ivBytes = stringToIV("CYRUS STUDIO    ");


    auto message_to_encode_length = env->GetArrayLength(data);
    auto *output = decodeByteArray(
            message_to_decode,
            static_cast<uint32_t>(message_to_encode_length),
            keyBytes,
            ivBytes,
            mode
    );

    env->ReleaseByteArrayElements(data, (jbyte *) message_to_decode, 0);

    // 释放资源
    delete[] keyBytes;
    delete[] ivBytes;

    auto iterations = message_to_encode_length / AES_BLOCKLEN;
    if (message_to_encode_length % AES_BLOCKLEN != 0) {
        iterations++;
    }
    return clientByteArrayFromUnit8(env, output, iterations * AES_BLOCKLEN * sizeof(uint8_t));
}


