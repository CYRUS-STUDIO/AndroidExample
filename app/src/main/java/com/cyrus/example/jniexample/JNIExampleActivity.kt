package com.cyrus.example.jniexample

import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.cyrus.example.R


class JNIExampleActivity : AppCompatActivity() {

    // 声明本地方法
    external fun intExample(value: Int): Int
    external fun doubleExample(value: Double): Double
    external fun booleanExample(value: Boolean): Boolean
    external fun stringExample(value: String?): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_jni_example)

        System.loadLibrary("native-lib"); // 加载 native 库

        // Int 参数示例
        findViewById<View>(R.id.button_int_example).setOnClickListener { v: View? ->
            val result = intExample(10)
            Toast.makeText(this, "Int Result: $result", Toast.LENGTH_SHORT).show()
        }

        // Double 参数示例
        findViewById<View>(R.id.button_double_example).setOnClickListener { v: View? ->
            val result = doubleExample(3.14)
            Toast.makeText(this, "Double Result: $result", Toast.LENGTH_SHORT).show()
        }

        // Boolean 参数示例
        findViewById<View>(R.id.button_boolean_example).setOnClickListener { v: View? ->
            val result = booleanExample(true)
            Toast.makeText(this, "Boolean Result: $result", Toast.LENGTH_SHORT).show()
        }

        // String 参数示例
        findViewById<View>(R.id.button_string_example).setOnClickListener { v: View? ->
            val result = stringExample("Hello JNI")
            Toast.makeText(this, "String Result: $result", Toast.LENGTH_SHORT).show()
        }

        // Array 参数示例
        findViewById<View>(R.id.button_array_example).setOnClickListener { v: View? ->
            val array = arrayOf(283, this, "HelloWorld")
            val result = JNIExample.arrayExample(array)
            Toast.makeText(this, "Array Example Result: $result", Toast.LENGTH_SHORT).show()
        }

        // 调用 Java 静态方法示例
        findViewById<View>(R.id.button_invoke_java_static_method).setOnClickListener { v: View? ->
            val result = JNIExample.invokeJavaStaticMethod()
            Toast.makeText(this, "invokeJavaStaticMethod Result: $result", Toast.LENGTH_SHORT).show()
        }
    }

}