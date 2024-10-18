cmake_minimum_required(VERSION 3.4.1)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the NDK library that you want CMake to locate.
        log)

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


add_library( # 库名称
        antidebug

        # 库类型
        SHARED

        # 源文件
        anti_debug.c)


target_link_libraries( # 绑定库到 log 库
        antidebug
        ${log-lib})
