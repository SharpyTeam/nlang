cmake_minimum_required(VERSION 3.10)
project(nlang_parser)

set(CMAKE_CXX_STANDARD 17)

set(NLANG_PARSER_SOURCES
        scanner.cpp)

set(NLANG_PARSER_HEADERS
        scanner.hpp
        parser.hpp
        char_stream.hpp)

add_library(nlang_parser ${NLANG_PARSER_HEADERS} ${NLANG_PARSER_SOURCES})
target_include_directories(nlang_parser PUBLIC .)
target_link_libraries(nlang_parser nlang_common)