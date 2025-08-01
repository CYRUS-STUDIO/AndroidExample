cmake_minimum_required(VERSION 3.4.1)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the NDK library that you want CMake to locate.
        log)

# 全局启用指令替换
add_definitions("-mllvm -sub")

## JNI示例 ##########################################################################################


add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)


target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

## 反调试 ###########################################################################################

add_library( # 库名称
        antidebug

        # 库类型
        SHARED

        # 源文件
        anti_debug.c)


target_link_libraries( # 绑定库到 log 库
        antidebug
        ${log-lib})


## 内联汇编 ##########################################################################################

add_library( # 设置库的名称
        assembly-lib

        # 设置库的类型
        SHARED

        # 设置源文件路径
        assembly-lib.cpp)

target_link_libraries( # 将 log 库链接到目标库
        assembly-lib
        ${log-lib})

## syscall ##########################################################################################

# 启用 C 和汇编语言的支持
enable_language(C ASM)

# 根据系统处理器架构选择不同的汇编文件
if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch")  # 检查当前系统是否为 AArch64 (ARM 64-bit) 架构
    # 为 `syscall64.s` 设置编译标志
    # `-x assembler-with-cpp` 表示使用 C 预处理器来编译汇编文件
    set_source_files_properties(syscall64.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

    # 添加一个共享库 (Shared Library)
    add_library(
            syscall-lib          # 设置库的名称为 `syscall-lib`

            SHARED               # 指定库的类型为共享库

            syscall64.s          # 添加 ARM64 汇编源文件

            syscall-lib.cpp      # 添加 C++ 源文件
    )

    # 如果系统处理器架构为 ARM (ARM 32-bit)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    # 为 `syscall32.s` 设置编译标志
    # `-x assembler-with-cpp` 表示使用 C 预处理器来编译汇编文件
    set_source_files_properties(syscall32.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

    # 添加一个共享库 (Shared Library)
    add_library(
            syscall-lib          # 设置库的名称为 `syscall-lib`

            SHARED               # 指定库的类型为共享库

            syscall32.s          # 添加 ARM 32 位汇编源文件

            syscall-lib.cpp      # 添加 C++ 源文件
    )
endif ()

target_link_libraries( # 将 log 库链接到目标库
        syscall-lib
        ${log-lib}
)

## OLLVM ##########################################################################################

add_library( # 设置库的名称
        ollvm-lib

        # 设置库的类型
        SHARED

        # 设置源文件路径
        ollvm-lib.cpp)

# 为 ollvm-lib 动态库启用虚假控制流
target_compile_options(
        ollvm-lib
        PRIVATE
        -mllvm -bcf)

target_link_libraries( # 将 log 库链接到目标库
        ollvm-lib
        ${log-lib})


## VMP ##########################################################################################

add_library( # 设置库的名称
        vmp-lib

        # 设置库的类型
        SHARED

        # 设置源文件路径
        vmp-lib.cpp)

target_link_libraries( # 将 log 库链接到目标库
        vmp-lib
        ${log-lib})

## Frida 反汇编 ##########################################################################################

add_library( # 设置库的名称
        frida_disassemble

        # 设置库的类型
        SHARED

        # 设置源文件路径
        frida_disassemble.cpp)

target_link_libraries( # 将 log 库链接到目标库
        frida_disassemble
        ${log-lib})


## Unicorn ##########################################################################################

add_library( # 设置库的名称
        unicorn

        # 设置库的类型
        SHARED

        # 设置源文件路径
        unicorn.cpp)

target_link_libraries( # 将 log 库链接到目标库
        unicorn
        ${log-lib})

## Unidbg ##########################################################################################

add_library( # 设置库的名称
        unidbg

        # 设置库的类型
        SHARED

        # 设置源文件路径
        unidbg.cpp)

target_link_libraries( # 将 log 库链接到目标库
        unidbg
        ${log-lib})

## Base64 ##########################################################################################

add_library( # 设置库的名称
        base64

        # 设置库的类型
        SHARED

        # 设置源文件路径
        base64.cpp
        custom_base64_alphabet.cpp
        dynamic_base64_alphabet.cpp)

target_link_libraries( # 将 log 库链接到目标库
        base64
        ${log-lib})


## CRC32 ##########################################################################################

add_library( # 设置库的名称
        crc32

        # 设置库的类型
        SHARED

        # 设置源文件路径
        crc32.cpp
        custom_crc32_table.cpp
        modified_crc32.cpp)

target_link_libraries( # 将 log 库链接到目标库
        crc32
        ${log-lib})

## MD5 ##########################################################################################

add_library( # 设置库的名称
        md5

        # 设置库的类型
        SHARED

        # 设置源文件路径
        md5.cpp
        md5_change_constant.cpp)

target_link_libraries( # 将 log 库链接到目标库
        md5
        ${log-lib})

## SHA1 ##########################################################################################

add_library( # 设置库的名称
        sha1

        # 设置库的类型
        SHARED

        # 设置源文件路径
        sha1.cpp
        sha1_jni.cpp
        sha1_change_constant.cpp)

target_link_libraries( # 将 log 库链接到目标库
        sha1
        ${log-lib})

## SHA256 ##########################################################################################

add_library( # 设置库的名称
        sha256

        # 设置库的类型
        SHARED

        # 设置源文件路径
        sha256.cpp)

target_link_libraries( # 将 log 库链接到目标库
        sha256
        ${log-lib})


## HMAC ##########################################################################################

add_library( # 设置库的名称
        hmac

        # 设置库的类型
        SHARED

        # 设置源文件路径
        hmac_md5.cpp
        hmac_sha256.cpp
)

target_link_libraries(
        hmac
        # 链接 log 库
        ${log-lib}
        # 链接 md5 库
        md5
        # 链接 sha256 库
        sha256)

## LibTomCrypt ##########################################################################################

# 添加 libtomcrypt 头文件路径
include_directories(libtomcrypt/src/headers)

# 查找 libtomcrypt 所有的 C 文件
file(GLOB LIBTOMCRYPT_SOURCES
        libtomcrypt/src/*.c
        libtomcrypt/src/misc/*.c
        libtomcrypt/src/misc/crypt/*.c
        libtomcrypt/src/ciphers/aes/*.c
        libtomcrypt/src/modes/cbc/*.c
        libtomcrypt/src/modes/cfb/*.c
        libtomcrypt/src/modes/ctr/*.c
        libtomcrypt/src/modes/ecb/*.c
)

# 编译 LibTomCrypt
add_library(libtomcrypt STATIC ${LIBTOMCRYPT_SOURCES})

## AES ##########################################################################################
add_library( # 设置库的名称
        aes

        # 设置库的类型
        SHARED

        # 设置源文件路径
        aes_jni.cpp
)

# 为 aes 动态库启用字符串加密
target_compile_options(
        aes
        PRIVATE
        -mllvm -sobf)

# 抹除符号
set_target_properties(aes PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/hide.map")

target_link_libraries(
        aes
        # 链接 log 库
        ${log-lib}
        # 链接 LibTomCrypt
        libtomcrypt
)

## CyrusStudioHook ##########################################################################################

find_package(shadowhook REQUIRED CONFIG)

add_library( # 设置库的名称
        cyrus_studio_hook

        # 设置库的类型
        SHARED

        # 设置源文件路径
        cyrus_studio_hook.cpp
)

# 为 cyrus_studio_hook 动态库启用字符串加密
target_compile_options(
        cyrus_studio_hook
        PRIVATE
        -mllvm -sobf)

# 抹除符号
set_target_properties(aes PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/hide.map")

target_link_libraries(
        cyrus_studio_hook
        # 链接 log 库
        ${log-lib}
        # 链接 shadowhook
        shadowhook::shadowhook
)


## FART ##########################################################################################

add_library( # 设置库的名称
        fart

        # 设置库的类型
        SHARED

        # 设置源文件路径
        fart.cpp
)

# 为 fart 动态库启用字符串加密
target_compile_options(
        fart
        PRIVATE
        -mllvm -sobf)

# 抹除符号
set_target_properties(fart PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/hide.map")

target_link_libraries(
        fart
        # 链接 log 库
        ${log-lib}
)


## so脱壳 ##########################################################################################

add_library( # 设置库的名称
        so_unpack

        # 设置库的类型
        SHARED

        # 设置源文件路径
        so_unpack/so_unpack.cpp
)

# 为 so_unpack 动态库启用字符串加密
target_compile_options(
        so_unpack
        PRIVATE
        -mllvm -sobf)

# 抹除符号
set_target_properties(so_unpack PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/hide.map")

target_link_libraries(
        so_unpack
        # 链接 log 库
        ${log-lib}
)

## so hooker ##########################################################################################

find_package(shadowhook REQUIRED CONFIG)

add_library(
        # 设置库的名称
        sohooker

        # 设置库的类型
        SHARED

        # 设置源文件路径
        sohooker.cpp
)

target_link_libraries(
        sohooker
        # 链接 log 库
        ${log-lib}
        # 链接 shadowhook
        shadowhook::shadowhook
)

