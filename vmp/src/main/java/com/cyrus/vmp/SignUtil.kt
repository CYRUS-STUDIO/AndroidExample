package com.cyrus.vmp

import java.security.MessageDigest
import java.util.Base64

object SignUtil {

    /**
     * 对输入字符串进行签名并返回 Base64 编码后的字符串
     * @param input 要签名的字符串
     * @return Base64 编码后的字符串
     */
    fun sign(input: String): String {
        // 使用 SHA-256 计算摘要
        val digest = MessageDigest.getInstance("SHA-256")
        val hash = digest.digest(input.toByteArray())

        // 使用 Base64 编码
        return Base64.getEncoder().encodeToString(hash)
    }
}

