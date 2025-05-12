package com.cyrus.example.aes


class NativeAESUtils {

    companion object {
        // 加载 native 库
        init {
            System.loadLibrary("aes")
        }

        // 声明 native 静态方法
        @JvmStatic
        external fun aesCBCEncode(data: ByteArray): ByteArray

        @JvmStatic
        external fun aesCBCDecode(data: ByteArray): ByteArray

        @JvmStatic
        external fun aesECBEncode(data: ByteArray): ByteArray

        @JvmStatic
        external fun aesECBDecode(data: ByteArray): ByteArray

        @JvmStatic
        external fun aesCTREncode(data: ByteArray): ByteArray

        @JvmStatic
        external fun aesCTRDecode(data: ByteArray): ByteArray
    }
}

// ByteArray 转 Hex 字符串
fun ByteArray.toHexString(): String {
    return joinToString("") { "%02x".format(it) }
}

// Hex 字符串 转 ByteArray
fun String.hexStringToByteArray(): ByteArray {
    require(length % 2 == 0) { "Hex string must have an even length" }
    return chunked(2).map { it.toInt(16).toByte() }.toByteArray()
}
