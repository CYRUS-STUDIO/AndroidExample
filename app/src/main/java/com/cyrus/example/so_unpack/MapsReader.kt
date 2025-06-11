package com.cyrus.example.so_unpack

import java.io.File

object MapsReader {

    /**
     * 读取 /proc/self/maps 文件，返回每一段内存映射的信息列表
     */
    fun readMaps(): List<MemoryMapEntry> {
        val result = mutableListOf<MemoryMapEntry>()

        try {
            val reader = File("/proc/self/maps").bufferedReader()
            reader.useLines { lines ->
                lines.forEach { line ->
                    val parts = line.trim().split(Regex("\\s+"), limit = 6)
                    if (parts.size < 5) return@forEach

                    val (addressRange, perms, offsetStr, dev, inodeStr) = parts
                    val path = if (parts.size == 6) parts[5] else null

                    val (startStr, endStr) = addressRange.split("-")
                    val startAddress = startStr.toLong(16)
                    val endAddress = endStr.toLong(16)
                    val offset = offsetStr.toLong(16)
                    val inode = inodeStr.toLong()

                    val entry = MemoryMapEntry(
                        startAddress = startAddress,
                        endAddress = endAddress,
                        permissions = perms,
                        offset = offset,
                        dev = dev,
                        inode = inode,
                        path = path
                    )
                    result.add(entry)
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }

        return result
    }
}
