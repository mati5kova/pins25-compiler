cmake_minimum_required(VERSION 3.30)
project(pins25_compiler C)

set(CMAKE_C_STANDARD 99)

add_executable(pins25_compiler src/main.c
        include/lexer.h
        src/lexer.c
)
