cmake_minimum_required(VERSION 3.26)
project(Client1)

set(CMAKE_CXX_STANDARD 17)

add_executable(Client2 main.cpp C:/Users/Admin/CLionProjects/ChatRoom/Server/common.cpp
        client.h
        client.cpp)
target_link_libraries(Client2 wsock32 ws2_32)
target_include_directories(Client2 PRIVATE C:/Users/Admin/CLionProjects/ChatRoom/Server)