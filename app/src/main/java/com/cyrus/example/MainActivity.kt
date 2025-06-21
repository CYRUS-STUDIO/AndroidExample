package com.cyrus.example

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.aes.AESActivity
import com.cyrus.example.antidebug.AntiDebug
import com.cyrus.example.antidebug.AntiDebugActivity
import com.cyrus.example.assembly.AssemblyActivity
import com.cyrus.example.base64.Base64Activity
import com.cyrus.example.classloader.ClassLoaderActivity
import com.cyrus.example.crc32.CRC32Activity
import com.cyrus.example.device_fingerprint.DeviceFingerprintActivity
import com.cyrus.example.fart.FartActivity
import com.cyrus.example.frida_disassemble.FridaDisassembleActivity
import com.cyrus.example.hmac.HMACActivity
import com.cyrus.example.hook.HookActivity
import com.cyrus.example.hotfix.HotFixActivity
import com.cyrus.example.jniexample.JNIExampleActivity
import com.cyrus.example.md5.MD5Activity
import com.cyrus.example.ollvm.OLLVMActivity
import com.cyrus.example.root.RootActivity
import com.cyrus.example.sha1.SHA1Activity
import com.cyrus.example.shell.DexExtractActivity
import com.cyrus.example.so_unpack.SoUnpackActivity
import com.cyrus.example.syscall.SyscallActivity
import com.cyrus.example.unicorn.UnicornActivity
import com.cyrus.example.unidbg.UnidbgActivity
import com.cyrus.example.vmp.VMPActivity


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // 获取按钮并设置点击事件
        val debugButton: Button = findViewById(R.id.debugButton)
        debugButton.setOnClickListener {
            // 点击按钮后跳转到 AntiDebugActivity
            val intent = Intent(this, AntiDebugActivity::class.java)
            startActivity(intent)
        }

        // JNI调用示例
        val jniExampleButton = findViewById<Button>(R.id.button_jni_example)
        jniExampleButton.setOnClickListener { v: View? ->
            val intent = Intent(
                this@MainActivity,
                JNIExampleActivity::class.java
            )
            startActivity(intent)
        }

        // SIGTRAP 反调试
        val sigtrapAntiDebugButton: Button = findViewById(R.id.sigtrapAntiDebugButton)
        sigtrapAntiDebugButton.setOnClickListener {
            val debuggerDetected = AntiDebug.isDebuggerDetected()
            if (debuggerDetected) {
                Toast.makeText(this, "Debugger Detected", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this, "No Debugger Detected", Toast.LENGTH_SHORT).show()
            }
        }

        // 汇编
        findViewById<Button>(R.id.button_assembly).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                AssemblyActivity::class.java
            )
            startActivity(intent)
        }

        // 系统调用 (syscall)
        findViewById<Button>(R.id.button_syscall).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                SyscallActivity::class.java
            )
            startActivity(intent)
        }

        // OLLVM
        findViewById<Button>(R.id.button_ollvm).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                OLLVMActivity::class.java
            )
            startActivity(intent)
        }

        // VMP
        findViewById<Button>(R.id.button_vmp).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                VMPActivity::class.java
            )
            startActivity(intent)
        }

        // Frida 反汇编
        findViewById<Button>(R.id.button_frida_disassemble).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                FridaDisassembleActivity::class.java
            )
            startActivity(intent)
        }

        // Unicorn
        findViewById<Button>(R.id.button_unicorn).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                UnicornActivity::class.java
            )
            startActivity(intent)
        }

        // Unidbg
        findViewById<Button>(R.id.button_unidbg).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                UnidbgActivity::class.java
            )
            startActivity(intent)
        }

        // Base64
        findViewById<Button>(R.id.button_base64).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                Base64Activity::class.java
            )
            startActivity(intent)
        }

        // CRC32
        findViewById<Button>(R.id.button_crc32).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                CRC32Activity::class.java
            )
            startActivity(intent)
        }

        // MD5
        findViewById<Button>(R.id.button_md5).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                MD5Activity::class.java
            )
            startActivity(intent)
        }

        // SHA1
        findViewById<Button>(R.id.button_sha1).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                SHA1Activity::class.java
            )
            startActivity(intent)
        }


        // HMAC
        findViewById<Button>(R.id.button_hmac).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                HMACActivity::class.java
            )
            startActivity(intent)
        }

        // AES
        findViewById<Button>(R.id.button_aes).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                AESActivity::class.java
            )
            startActivity(intent)
        }

        // ClassLoader
        findViewById<Button>(R.id.button_classloader).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                ClassLoaderActivity::class.java
            )
            startActivity(intent)
        }

        // 热修复
        findViewById<Button>(R.id.button_hotfix).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                HotFixActivity::class.java
            )
            startActivity(intent)
        }

        // Hook
        findViewById<Button>(R.id.button_hook).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                HookActivity::class.java
            )
            startActivity(intent)
        }

        // Root
        findViewById<Button>(R.id.button_root).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                RootActivity::class.java
            )
            startActivity(intent)
        }

        // 函数抽取壳
        findViewById<Button>(R.id.button_dex_extract).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                DexExtractActivity::class.java
            )
            startActivity(intent)
        }

        // FART
        findViewById<Button>(R.id.button_fart).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                FartActivity::class.java
            )
            startActivity(intent)
        }


        // so 脱壳
        findViewById<Button>(R.id.button_so_shell).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                SoUnpackActivity::class.java
            )
            startActivity(intent)
        }

        // 设备指纹
        findViewById<Button>(R.id.button_device_fingerprint).setOnClickListener {
            val intent = Intent(
                this@MainActivity,
                DeviceFingerprintActivity::class.java
            )
            startActivity(intent)
        }
    }
}
