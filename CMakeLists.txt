cmake_minimum_required(VERSION 3.10)
project(mymuduo)

# 设置源文件路径
file(GLOB all_src_files 
    ${CMAKE_CURRENT_SOURCE_DIR}/muduo/base/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/muduo/net/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/muduo/test/main_test.cpp 
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 11)

# 查找线程库
find_package(Threads REQUIRED)

# 测试文件输出
add_executable(test.out ${all_src_files})

# 添加头文件目录
target_include_directories(test.out PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/muduo/base
    ${CMAKE_CURRENT_SOURCE_DIR}/muduo/net
)

# 链接线程库
target_link_libraries(test.out PRIVATE Threads::Threads)