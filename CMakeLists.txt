cmake_minimum_required(VERSION 3.28)
project(mini_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(mini_cpp
        main.cpp
        lexer.h
        parser.h)
