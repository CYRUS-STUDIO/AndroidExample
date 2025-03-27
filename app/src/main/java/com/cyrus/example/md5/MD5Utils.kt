package com.cyrus.example.md5

import java.security.MessageDigest

class MD5Utils {

    companion object {
        // 加载 native 库
        init {
            System.loadLibrary("md5")
        }

        // 声明 native 静态方法
        @JvmStatic
        external fun md5(input: String): String
        @JvmStatic
        external fun changeMD5Init(input: String): String
        @JvmStatic
        external fun changeMD5Update(input: String): String
        @JvmStatic
        external fun changeConstant(input: String): String


        @JvmStatic
        fun javaMD5(input: String): String {
            val md = MessageDigest.getInstance("MD5")
            val digest = md.digest(input.toByteArray())
            return digest.joinToString("") { "%02x".format(it) }
        }
    }

}