package com.cyrus.example.vmp

import android.content.Context
import java.io.InputStream
import javax.crypto.Cipher
import javax.crypto.SecretKey
import javax.crypto.spec.SecretKeySpec

object AESUtils {
    private const val ALGORITHM = "AES"
    private const val TRANSFORMATION = "AES/ECB/PKCS5Padding"

    // 从 assets 中读取文件并解密
    fun decryptFileFromAssets(context: Context, vmpFileName: String, keyFileName: String): ByteArray? {
        // 读取密钥文件
        val key = loadKeyFromAssets(context, keyFileName)

        // 读取加密的 vmp 文件
        val encryptedData = readFileFromAssets(context, vmpFileName)

        // 解密
        return decrypt(encryptedData, key)
    }

    // 读取文件内容为字节数组
    private fun readFileFromAssets(context: Context, fileName: String): ByteArray {
        val inputStream: InputStream = context.assets.open(fileName)
        return inputStream.readBytes()
    }

    // 从 assets 中加载密钥文件
    private fun loadKeyFromAssets(context: Context, keyFileName: String): SecretKey {
        val keyBytes = readFileFromAssets(context, keyFileName)
        return SecretKeySpec(keyBytes, ALGORITHM)
    }

    // 解密
    private fun decrypt(data: ByteArray, key: SecretKey): ByteArray {
        val cipher = Cipher.getInstance(TRANSFORMATION)
        cipher.init(Cipher.DECRYPT_MODE, key)
        return cipher.doFinal(data)
    }
}
