cmake_minimum_required(VERSION 3.26)
project(Client3)

set(CMAKE_CXX_STANDARD 17)

add_executable(Client3 main.cpp C:/Users/Admin/CLionProjects/ChatRoom/Server/common.cpp
        client.h
        client.cpp)
target_link_libraries(Client3 wsock32 ws2_32)
target_include_directories(Client3 PRIVATE C:/Users/Admin/CLionProjects/ChatRoom/Server)