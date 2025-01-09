package com.cyrus.example.vmp

class SimpleVMP {

    companion object {
        // 加载本地库
        init {
            System.loadLibrary("vmp-lib")
        }

        // 定义静态方法 execute
        @JvmStatic
        external fun execute(bytecode: ByteArray, input: String): String
    }

}
