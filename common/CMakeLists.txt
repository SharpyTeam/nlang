cmake_minimum_required(VERSION 3.10)
project(nlang_common)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED TRUE)

set(NLANG_COMMON_SOURCES
        stub.cpp
        bytecode.cpp)

set(NLANG_COMMON_HEADERS
        token.hpp
        ast.hpp
        opcodes.hpp
        bytecode.hpp
        conversion_utils.hpp)

add_library(nlang_common ${NLANG_COMMON_HEADERS} ${NLANG_COMMON_SOURCES})
target_include_directories(nlang_common PUBLIC .)