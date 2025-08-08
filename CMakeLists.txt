cmake_minimum_required(VERSION 3.30)
project(pins25_compiler C)

set(CMAKE_C_STANDARD 99)

add_executable(pins25_compiler src/main.c
        include/lexer.h
        src/lexer.c
        include/options.h
        src/options.c
        include/error_utils.h
        src/error_utils.c
        src/parser.c
        include/parser.h
        include/ast.h
        src/ast.c
        include/token_stream.h
        src/token_stream.c
        include/compiler_data.h
        src/compiler_data.c
        include/token.h
)
