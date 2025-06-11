package com.cyrus.example.so_unpack

data class MemoryMapEntry(
    val startAddress: Long,
    val endAddress: Long,
    val permissions: String,
    val offset: Long,
    val dev: String,
    val inode: Long,
    val path: String?
)
