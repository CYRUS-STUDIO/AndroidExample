package com.cyrus.example.md5

import com.cyrus.example.dex2c.Dex2C
import java.security.MessageDigest

object MD5Tools {

    @Dex2C
    fun javaMD5(input: String): String {
        val md = MessageDigest.getInstance("MD5")
        val digest = md.digest(input.toByteArray())
        return digest.joinToString("") { "%02x".format(it) }
    }

}