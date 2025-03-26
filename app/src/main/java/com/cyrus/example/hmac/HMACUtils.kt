package com.cyrus.example.hmac

import javax.crypto.Mac
import javax.crypto.spec.SecretKeySpec


class HMACUtils {

    companion object {
        // 加载 native 库
        init {
            System.loadLibrary("hmac")
        }

        // 声明 native 静态方法
        @JvmStatic
        external fun hmacMD5(input: String): String
        @JvmStatic
        external fun hmacSHA256(input: String): String

        @JvmStatic
        fun javaHmacMD5(data: String, key: String): String {
            val secretKeySpec = SecretKeySpec(key.toByteArray(), "HmacMD5")
            val mac = Mac.getInstance("HmacMD5")
            mac.init(secretKeySpec)
            val hashBytes = mac.doFinal(data.toByteArray())
            return hashBytes.joinToString("") { "%02x".format(it) }
        }

        @JvmStatic
        fun javaHmacSha256(data: String, key: String): String {
            val secretKeySpec = SecretKeySpec(key.toByteArray(), "HmacSHA256")
            val mac = Mac.getInstance("HmacSHA256")
            mac.init(secretKeySpec)
            val hashBytes = mac.doFinal(data.toByteArray())
            return hashBytes.joinToString("") { "%02x".format(it) }
        }

    }

}