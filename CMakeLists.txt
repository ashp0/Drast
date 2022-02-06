cmake_minimum_required(VERSION 3.13)
project(drast C)
set(CMAKE_C_STANDARD 11)
add_executable(drast src/main.c src/argument_parser.c src/argument_parser.h) 