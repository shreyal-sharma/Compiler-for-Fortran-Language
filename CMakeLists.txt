cmake_minimum_required(VERSION 3.27)
project(CS280ParserProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(CS280ParserProject main.cpp
        lex.cpp
        lex.h
        parser.cpp
        parser.h
        prog2.cpp
)
