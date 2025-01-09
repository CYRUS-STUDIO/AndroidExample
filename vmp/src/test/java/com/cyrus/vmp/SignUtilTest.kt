package com.cyrus.vmp


import org.junit.Test
import org.junit.Assert.*

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
class SignUtilTest {
    @Test
    fun sign() {
        val input = "example"
        val signed = SignUtil.sign(input)
        println("Input: $input")
        println("Signed: $signed")

        assertEquals("UNhY4JhezH9gQYqvDMWrWH9CwlcKiECVqejMrND2VFw=", signed)
    }
}