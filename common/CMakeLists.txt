cmake_minimum_required(VERSION 3.10)
project(nlang_common)

set(CMAKE_CXX_STANDARD 17)

set(NLANG_COMMON_SOURCES
        stub.cpp)

set(NLANG_COMMON_HEADERS
        token.hpp
        ast.hpp
        conversion_utils.hpp
        error_reporter.hpp
        utils/defs.hpp
        utils/traits.hpp
        utils/nan_boxed_primitive.hpp)

add_library(nlang_common ${NLANG_COMMON_HEADERS} ${NLANG_COMMON_SOURCES})
target_include_directories(nlang_common PUBLIC .)