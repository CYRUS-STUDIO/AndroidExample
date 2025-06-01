package com.cyrus.example.fart

object AntiFART {

    private val TAG = AntiFART.javaClass.simpleName

    init {
        System.loadLibrary("fart")
    }

    /**
     * 尝试通过反射获取类中所有方法判断是否有 fart 特征
     *
     * 但是 Android 系统自 Android 9 (API 28) 开始启用了 Hidden API enforcement，禁止 app 访问某些内部 API，尤其是通过反射或 JNI 获取非 SDK 公共接口。
     *
     * 所以该方案行不通！！！
     *
     * https://developer.android.com/guide/app-compatibility/restrictions-non-sdk-interfaces?hl=zh-cn
     *
     */
    @JvmStatic
    external fun detectFARTByReflection(): Boolean

    /**
     * 通过反射获取指定类中所有方法签名
     */
    @JvmStatic
    external fun dumpMethods(className: String): String

    /**
     * 读取 /proc/self/maps 文件，获取当前 app 已加载的所有文件
     */
    @JvmStatic
    external fun listLoadedFiles(): Array<String>

    /**
     * 判断指定 so 中是否存在指定符号
     */
    @JvmStatic
    external fun hasSymbolInSO(soName: String?, symbolName: String?): Boolean

    /**
     *
     * 尝试通过 dlsym 查找 so 中的 fart 特征符号
     *
     * 报错如下：
     * library "/apex/com.android.runtime/lib64/libartpalette.so" ("/apex/com.android.runtime/lib64/libartpalette.so") needed or dlopened by "/data/app/com.cyrus.example-UyZUbCRlV7pfO_A3DsN3Sw==/base.apk!/lib/arm64-v8a/libfart.so" is not accessible for the namespace: [name="classloader-namespace", ld_library_paths="", default_library_paths="/data/app/com.cyrus.example-UyZUbCRlV7pfO_A3DsN3Sw==/lib/arm64:/data/app/com.cyrus.example-UyZUbCRlV7pfO_A3DsN3Sw==/base.apk!/lib/arm64-v8a", permitted_paths="/data:/mnt/expand:/data/data/com.cyrus.example"]
     *
     * 当前报错的内容说明：
     * 1. 你的 libfart.so 依赖于 /apex/com.android.runtime/lib64/libartpalette.so（由 dlsym 或依赖库触发）
     * 2. 但当前 classloader namespace（App 的 native 库加载空间）中并没有权限访问这个路径
     *
     * 所以该方案行不通！！！
     *
     */
    @JvmStatic
    fun detectFartDlsym(): MutableList<Pair<String, String>> {
        // 定义 FART 特征符号
        val fartSymbols = listOf(
            "dumpDexFileByExecute",
            "dumpArtMethod",
            "myfartInvoke",
            "DexFile_dumpMethodCode"
        )

        /**
         * 扫描所有已加载的 so 文件，判断是否包含 FART 特征符号
         */
        val matches = mutableListOf<Pair<String, String>>()
        val loadedFiles = listLoadedFiles()

        loadedFiles.filter { it.endsWith(".so") }.forEach { soPath ->
            fartSymbols.forEach { symbol ->
                val found = hasSymbolInSO(soPath, symbol)
                if (found) {
                    matches.add(soPath to symbol)
                }
            }
        }

        return matches
    }

    /**
     * FART特征检测
     *
     * 1. 检测已加载的 .so 文件；
     * 2. 读取其二进制内容；
     * 3. 用模糊匹配检测是否包含黑名单中的函数特征名；
     * 4. 返回命中的 so 路径（String[]）。
     */
    @JvmStatic
    external fun detectFartInLoadedSO(): Array<String>

    @JvmStatic
    external fun detectFartInLoadedDex(): Array<String>

}
