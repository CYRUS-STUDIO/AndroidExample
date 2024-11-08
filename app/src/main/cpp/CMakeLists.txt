cmake_minimum_required(VERSION 3.4.1)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the NDK library that you want CMake to locate.
        log)

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
