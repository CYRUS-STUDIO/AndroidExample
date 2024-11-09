    .text                      // 表示接下来的代码段是可执行代码段
    .global raw_syscall        // 将 `raw_syscall` 设为全局符号，使其可以被其他文件引用
    .type raw_syscall, @function // 指定 `raw_syscall` 是一个函数

raw_syscall:
        // 将第一个参数 (系统调用号) 传递给 X8 寄存器
        MOV             X8, X0    // X8 = X0, 系统调用号存储在 X8 中

        // 将其余的参数从 X1-X6 依次向前移动一位 (为系统调用准备参数)
        MOV             X0, X1    // X0 = X1, 系统调用的第一个参数
        MOV             X1, X2    // X1 = X2, 系统调用的第二个参数
        MOV             X2, X3    // X2 = X3, 系统调用的第三个参数
        MOV             X3, X4    // X3 = X4, 系统调用的第四个参数
        MOV             X4, X5    // X4 = X5, 系统调用的第五个参数
        MOV             X5, X6    // X5 = X6, 系统调用的第六个参数

        // 使用 SVC 指令触发系统调用 (Supervisor Call)
        SVC             0         // 发起系统调用，中断进入内核态执行

        RET                      // 返回
