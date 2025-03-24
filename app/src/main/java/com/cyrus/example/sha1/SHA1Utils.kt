package com.cyrus.example.sha1

import android.R
import java.security.MessageDigest


class SHA1Utils {

    companion object {
        // 加载 native 库
        init {
            System.loadLibrary("sha1")
        }

        // 声明 native 静态方法
        @JvmStatic
        external fun sha1(input: String): String
        @JvmStatic
        external fun changeSHA1Init(input: String): String
        @JvmStatic
        external fun changeSHA1Update(input: String): String
        @JvmStatic
        external fun changeConstant(input: String): String


        @JvmStatic
        fun javaSHA1(input: String): String {
            // 获取 SHA1 实例
            val md = MessageDigest.getInstance("SHA-1")
            // 更新数据
            md.update(input.toByteArray())
            // 计算哈希
            val digest = md.digest()
            // 将哈希转换为十六进制字符串
            return digest.joinToString("") { "%02x".format(it) }
        }
    }

}