cmake_minimum_required(VERSION 3.10)
project(nlang_interpreter)

set(CMAKE_CXX_STANDARD 17)

set(NLANG_INTERPRETER_SOURCES
        stub.cpp)

set(NLANG_INTERPRETER_HEADERS
        ast_interpreter.hpp
        heap.hpp
        value.hpp)

add_library(nlang_interpreter ${NLANG_INTERPRETER_HEADERS} ${NLANG_INTERPRETER_SOURCES})
target_include_directories(nlang_interpreter PUBLIC .)
target_link_libraries(nlang_interpreter nlang_common)
target_link_libraries(nlang_interpreter nlang_parser)