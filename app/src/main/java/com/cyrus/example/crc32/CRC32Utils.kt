package com.cyrus.example.crc32

class CRC32Utils {

    companion object {
        // 加载 native 库
        init {
            System.loadLibrary("crc32")
        }

        // 声明 native 静态方法
        @JvmStatic
        external fun crc32(input: String): String
        @JvmStatic
        external fun customTableCRC32(input: String): String
        @JvmStatic
        external fun modifiedCRC32(input: String): String
    }

}