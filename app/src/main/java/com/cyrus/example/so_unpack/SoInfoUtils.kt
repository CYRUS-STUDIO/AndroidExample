package com.cyrus.example.so_unpack

object SoInfoUtils {
    init {
        System.loadLibrary("so_unpack")
    }

    /**
     * 调用 JNI 层方法，枚举当前进程中已加载的 so 模块名称列表
     * @return 已加载模块名字符串数组，找不到或失败返回 null
     */
    external fun enumerateSoModules(): Array<String>?
}
