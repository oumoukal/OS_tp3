cmake_minimum_required(VERSION 3.29)
project(OS_TP3)

set(CMAKE_CXX_STANDARD 17)

add_executable(OS_TP3 main.cpp
        FileSystem.cpp
        BlockDevice.cpp)
