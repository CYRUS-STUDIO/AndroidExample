package com.cyrus.example.so_unpack

object MemoryProtector {

    init {
        System.loadLibrary("so_unpack")
    }

    // 权限常量，与系统定义一致（mman-common.h）
    const val PROT_NONE  = 0x0
    const val PROT_READ  = 0x1
    const val PROT_WRITE = 0x2
    const val PROT_EXEC  = 0x4

    /**
     * 修改内存页权限
     * @param address 起始地址
     * @param size 大小（字节数）
     * @param prot 保护类型（可组合：PROT_READ or PROT_WRITE or PROT_EXEC）
     * @return 0 表示成功，非0表示失败
     */
    external fun protect(address: Long, size: Long, prot: Int): Int
}
