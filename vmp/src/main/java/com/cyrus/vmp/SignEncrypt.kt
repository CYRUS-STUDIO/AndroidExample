package com.cyrus.vmp

import java.io.File

fun main() {
    // 获取工程根目录路径
    val projectRoot = System.getProperty("user.dir")

    // 设置相对路径
    val encryptedFile = File(projectRoot, "vmp/sign/sign.vmp") // 相对路径
    val keyFile = File(projectRoot, "vmp/sign/sign.key") // 相对路径

    // 输入文件路径
    val inputFile = File(projectRoot, "vmp/sign/sign") // 需要加密的文件


    try {
        // 使用 AES 加密文件
        AESUtils.encryptFile(inputFile, encryptedFile, keyFile)
        println("File encryption completed, saved as: ${encryptedFile.absolutePath}")
        println("Key saved as: ${keyFile.absolutePath}")
    } catch (e: Exception) {
        e.printStackTrace()
    }
}
