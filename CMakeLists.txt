cmake_minimum_required(VERSION 3.21)
project(drast)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        src/compiler/Lexer/Lexer.h
        src/compiler/Lexer/Lexer.cpp
        src/compiler/Lexer/Token.h
        src/compiler/Lexer/TokenUtils.h
        src/compiler/Lexer/Token.cpp

        src/compiler/Parser/Parser.h
        src/compiler/Parser/Parser.cpp

        src/compiler/AST/AST.h
        src/compiler/AST/AST.cpp

        src/compiler/SemanticAnalyzer/SemanticAnalyzer.h
        src/compiler/SemanticAnalyzer/SemanticAnalyzer.cpp

        src/compiler/Utils/Utils.h
        src/compiler/Utils/Characters.cpp
        src/compiler/Utils/File.cpp

        src/compiler/Common/Error.cpp
        src/compiler/Common/Error.h
        src/compiler/Common/LookupTable.h

        src/main.cpp)

add_executable(drast ${SOURCES})