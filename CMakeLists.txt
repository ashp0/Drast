cmake_minimum_required(VERSION 3.21)
project(drast)

set(CMAKE_CXX_STANDARD 20)

add_executable(drast
        src/main.cpp
        src/compiler/Lexer.cpp
        src/compiler/Lexer.h
        src/compiler/Token.cpp
        src/compiler/Token.h
        src/compiler/AST.cpp
        src/compiler/AST.h
        src/compiler/Parser.cpp
        src/compiler/Parser.h
        src/compiler/SemanticAnalyzer.cpp
        src/compiler/SemanticAnalyzer.h
        src/compiler/Error.cpp
        src/compiler/Error.h
        src/compiler/Utils.cpp
        src/compiler/Utils.h
        src/compiler/LookupTable.h)