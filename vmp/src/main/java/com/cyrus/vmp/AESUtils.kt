package com.cyrus.vmp

import java.io.ByteArrayOutputStream
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.SecretKeySpec

object AESUtils {

    private const val ALGORITHM = "AES"
    private const val TRANSFORMATION = "AES/ECB/PKCS5Padding" // AES 加密模式

    // 生成一个 128 位的 AES 密钥
    fun generateSecretKey(): SecretKey {
        val keyGenerator = KeyGenerator.getInstance(ALGORITHM)
        keyGenerator.init(128) // AES 128 位
        return keyGenerator.generateKey()
    }

    // 使用给定的密钥加密数据
    fun encrypt(data: ByteArray, key: SecretKey): ByteArray {
        val cipher = Cipher.getInstance(TRANSFORMATION)
        cipher.init(Cipher.ENCRYPT_MODE, key)
        return cipher.doFinal(data)
    }

    // 使用给定的密钥解密数据
    fun decrypt(data: ByteArray, key: SecretKey): ByteArray {
        val cipher = Cipher.getInstance(TRANSFORMATION)
        cipher.init(Cipher.DECRYPT_MODE, key)
        return cipher.doFinal(data)
    }

    // 将文件内容加密并导出到新文件
    fun encryptFile(inputFile: File, outputFile: File, keyFile: File) {
        // 读取文件内容
        val fileData = readFile(inputFile)

        // 生成密钥
        val secretKey = generateSecretKey()

        // 加密文件内容
        val encryptedData = encrypt(fileData, secretKey)

        // 保存加密后的数据到新文件（.vmp 文件）
        writeFile(outputFile, encryptedData)

        // 保存密钥到文件
        saveKeyToFile(secretKey, keyFile)
    }

    // 解密文件内容并导出到新文件
    fun decryptFile(inputFile: File, outputFile: File, keyFile: File) {
        // 从文件加载密钥
        val secretKey = loadKeyFromFile(keyFile)

        // 读取加密后的文件内容
        val encryptedData = readFile(inputFile)

        // 解密文件内容
        val decryptedData = decrypt(encryptedData, secretKey)

        // 保存解密后的数据到文件
        writeFile(outputFile, decryptedData)
    }

    // 读取文件内容并返回字节数组
    fun readFile(file: File): ByteArray {
        val fis = FileInputStream(file)
        val baos = ByteArrayOutputStream()
        val buffer = ByteArray(1024)
        var bytesRead: Int
        while (fis.read(buffer).also { bytesRead = it } != -1) {
            baos.write(buffer, 0, bytesRead)
        }
        fis.close()
        return baos.toByteArray()
    }

    // 将字节数组写入到文件
    fun writeFile(file: File, data: ByteArray) {
        val fos = FileOutputStream(file)
        fos.write(data)
        fos.close()
    }

    // 保存密钥到文件
    private fun saveKeyToFile(key: SecretKey, keyFile: File) {
        val fos = FileOutputStream(keyFile)
        fos.write(key.encoded)
        fos.close()
    }

    // 从文件加载密钥
    fun loadKeyFromFile(keyFile: File): SecretKey {
        val keyBytes = ByteArray(keyFile.length().toInt())
        val fis = FileInputStream(keyFile)
        fis.read(keyBytes)
        fis.close()
        return SecretKeySpec(keyBytes, ALGORITHM)
    }

}
