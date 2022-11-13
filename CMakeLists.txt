cmake_minimum_required(VERSION 3.23)
project(chatroom)

set(CMAKE_CXX_STANDARD 23)

add_executable(chatroom

        chatroom main.cpp
        src/server/server.cpp

        headers/server/server.hpp)
