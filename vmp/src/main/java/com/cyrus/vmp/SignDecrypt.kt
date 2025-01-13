package com.cyrus.vmp

import com.cyrus.vmp.AESUtils.loadKeyFromFile
import com.cyrus.vmp.AESUtils.readFile
import com.cyrus.vmp.AESUtils.writeFile
import java.io.File


fun main() {

    // 获取工程根目录路径
    val projectRoot = System.getProperty("user.dir")

    // 输入加密文件路径
    val encryptedFile = File(projectRoot, "vmp/sign/sign.vmp")

    // 密钥文件路径
    val keyFile = File(projectRoot, "vmp/sign/sign.key")

    // 输出解密文件路径
    val decryptedFile = File(projectRoot, "vmp/sign/sign_")

    try {
        // 从文件加载密钥
        val secretKey = loadKeyFromFile(keyFile)

        // 解密文件
        val encryptedData = readFile(encryptedFile)
        val decryptedData: ByteArray = AESUtils.decrypt(encryptedData, secretKey)

        // 保存解密后的文件
        writeFile(decryptedFile, decryptedData)
        println("File decryption completed, saved as: ${decryptedFile.absolutePath}")
    } catch (e: Exception) {
        e.printStackTrace()
    }
}
