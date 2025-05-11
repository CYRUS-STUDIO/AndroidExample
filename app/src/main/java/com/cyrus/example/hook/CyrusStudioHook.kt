package com.cyrus.example.hook

import com.bytedance.shadowhook.ShadowHook
import com.bytedance.shadowhook.ShadowHook.ConfigBuilder

object CyrusStudioHook {

    init {
        System.loadLibrary("cyrus_studio_hook") // 加载 native 实现
    }

    fun init(){
        // 初始化 ShadowHook
        ShadowHook.init(
            ConfigBuilder()
                // 设置 hook 模式：
                // UNIQUE 模式：同一个 hook 点只能被 hook 一次（unhook 后可以再次 hook）。
                // SHARED 模式：可对同一个 hook 点并发执行多个 hook 和 unhook，彼此互不干扰。
                .setMode(ShadowHook.Mode.UNIQUE)

                // 启用调试日志，方便开发阶段查看 hook 的行为
                .setDebuggable(true)

                // 启用 hook 记录功能，可以记录每一次 hook 的详细信息，开发调试有用（建议发布时设为 false）
                .setRecordable(true)

                // 构建配置对象
                .build()
        )
    }


    @JvmStatic
    external fun hookLoadMethod()

    @JvmStatic
    external fun hookExecve()

    /**
     * 使用 execve 调用 dex2oat 对指定 dex 进行优化
     *
     * 注意！！！ 需要 root 环境 + 修改 SELinux 策略 ！！！
     *
     * @param dexPath 要优化的 .dex 文件路径（如 /data/local/tmp/classes.dex）
     * @param oatPath 优化输出的 .odex 文件路径（如 /data/local/tmp/classes.odex）
     * @return true 表示调用成功（不代表 dex2oat 成功执行）
     */
    @JvmStatic
    external fun dex2oat(dexPath: String, oatPath: String): Boolean

}
