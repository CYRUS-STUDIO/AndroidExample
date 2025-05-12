package com.cyrus.example.root

import com.topjohnwu.superuser.Shell

object RootManager {

    fun init(){
        // Set settings before the main shell can be created
        Shell.enableVerboseLogging = true
        Shell.setDefaultBuilder(
            Shell.Builder.create()
                .setFlags(Shell.FLAG_MOUNT_MASTER)
                .setInitializers(Shell.Initializer::class.java)
                .setTimeout(10)
        )
    }

    fun runRootCommand() {
        val result = Shell.cmd("ls /data").exec()

        if (result.isSuccess) {
            // 输出命令执行结果
            for (line in result.out) {
                println("Output: $line")
            }
        } else {
            println("Command failed: ${result.err}")
        }
    }

}