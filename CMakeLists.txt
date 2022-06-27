cmake_minimum_required(VERSION 3.18)
project(ThreadPool)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Win MSVC下需要手动指定源码编码类型
if (MSVC)
    add_compile_options("/source-charset:utf-8")
endif()

add_executable(${PROJECT_NAME} main.cpp ThreadPool.hpp)

# 如果没有下面这个链接库，则会报错
# undefined reference to `pthread_create'
# 原因在于，linux下，C++ std::thread 是对pthread的封装
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries (${PROJECT_NAME} pthread)
endif()
