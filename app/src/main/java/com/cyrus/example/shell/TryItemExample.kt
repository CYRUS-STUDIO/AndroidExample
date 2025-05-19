package com.cyrus.example.shell

class TryItemExample {

    // ✅ 0. 没有 try-catch（不会生成 TryItem）
    fun noTryCatch(): Int {
        val a = 1
        val b = 2
        return a + b
    }

    // ✅ 1. 简单 try-catch（一个 TryItem）
    fun simpleTryCatch(): String {
        return try {
            val x = 10 / 2
            "Result: $x"
        } catch (e: Exception) {
            "Caught Exception"
        }
    }

    // ✅ 2. 多个 catch 分支（一个 TryItem，多个 handler entry）
    fun multiCatch(input: String?): Int {
        return try {
            input!!.length
        } catch (e: NullPointerException) {
            -1
        } catch (e: Exception) {
            -2
        }
    }

    // ✅ 3. try-catch-finally（一个 TryItem + finally handler）
    fun tryCatchFinally(): Int {
        return try {
            1 / 0
        } catch (e: ArithmeticException) {
            -100
        } finally {
            println("finally block executed")
        }
    }

    // ✅ 4. 嵌套 try-catch（两个 TryItem，嵌套结构）
    fun nestedTryCatch(): String {
        return try {
            try {
                val data = "123".toInt()
                "Parsed: $data"
            } catch (e: NumberFormatException) {
                "Inner Catch"
            }
        } catch (e: Exception) {
            "Outer Catch"
        }
    }

    // ✅ 5. 只有 finally，无 catch（一个 TryItem，无 handler entry）
    fun onlyFinally(): Int {
        try {
            val x = 1 + 1
        } finally {
            println("executing finally without catch")
        }
        return 0
    }

    // ✅ 6. 多个独立 try 块（多个 TryItem，非嵌套）
    fun multipleTryBlocks(): Int {
        try {
            val x = 10 / (1 - 1) // 故意制造除零异常，确保不会被优化
        } catch (e: Exception) {
            println("First catch")
        }

        try {
            val b = "abc".toInt() // 会抛出 NumberFormatException
        } catch (e: NumberFormatException) {
            println("Second catch")
        }

        return 0
    }
}
