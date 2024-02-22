cmake_minimum_required(VERSION 3.26)
project(Server)

set(CMAKE_CXX_STANDARD 17)

add_executable(Server main.cpp
        common.h
        common.cpp
        server.h
        server.cpp)
target_link_libraries(Server wsock32 ws2_32)
