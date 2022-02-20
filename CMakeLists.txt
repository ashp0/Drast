cmake_minimum_required(VERSION 3.13)
project(drast C)
set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_executable(drast src/main.c
        src/argument_parser.c
        src/argument_parser.h

        src/compiler/lexer.c
        src/compiler/lexer.h
        src/compiler/token.c
        src/compiler/token.h
        src/compiler/ast.c
        src/compiler/ast.h
        src/compiler/parser.c
        src/compiler/parser.h
        src/compiler/ast_print.c
        src/compiler/ast_print.h
        )