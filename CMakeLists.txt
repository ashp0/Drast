cmake_minimum_required(VERSION 3.22)
project(drast)

set(CMAKE_CXX_STANDARD 17)

add_executable(drast
        src/main.cpp

        # ArgsParser
        src/ArgsParser/ArgsParser.cpp
        src/ArgsParser/ArgsParser.h

        # Common
        src/Common/FileUtils.h
        src/Common/Location.h
        src/Common/Report.h

        # Config
        src/Config/Config.cpp
        src/Config/Config.h

        # Lexer
        src/Lexer/Lexer.cpp
        src/Lexer/Lexer.h
        src/Lexer/Token.cpp
        src/Lexer/Token.h

        # Node
        src/Node/Node.cpp
        src/Node/Node.h

        # Parser
        src/Parser/Parser.cpp
        src/Parser/Parser.h

        # TypeCheck
        src/TypeCheck/TypeChecker.cpp
        src/TypeCheck/TypeChecker.h)