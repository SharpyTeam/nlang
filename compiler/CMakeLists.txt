cmake_minimum_required(VERSION 3.10)
project(nlang_compiler)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED TRUE)

set(NLANG_COMPILER_SOURCES
        compiler.cpp
        assembler.cpp
        disassembler.cpp)

set(NLANG_COMPILER_HEADERS
        compiler.hpp
        assembler.hpp
        disassembler.hpp)

add_library(nlang_compiler ${NLANG_COMPILER_HEADERS} ${NLANG_COMPILER_SOURCES})
target_include_directories(nlang_compiler PUBLIC .)
target_link_libraries(nlang_compiler nlang_common)