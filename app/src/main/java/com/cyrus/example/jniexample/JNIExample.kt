package com.cyrus.example.jniexample

class JNIExample {

    companion object {
        init {
            System.loadLibrary("native-lib") // 加载 native 库
        }

        @JvmStatic
        external fun arrayExample(array: Array<Any>): Any
    }
}
