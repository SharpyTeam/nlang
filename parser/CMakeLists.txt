cmake_minimum_required(VERSION 3.10)
project(nlang_parser)

set(CMAKE_CXX_STANDARD 17)

set(NLANG_PARSER_SOURCES
        src/scanner.cpp)

set(NLANG_PARSER_HEADERS
        include/parser/scanner.hpp
        include/parser/parser.hpp
        include/parser/char_stream.hpp
        include/parser/scanner_new.hpp
        include/parser/token_stream.hpp include/parser/stream_cache.hpp)

add_library(nlang_parser ${NLANG_PARSER_HEADERS} ${NLANG_PARSER_SOURCES})
target_include_directories(nlang_parser PUBLIC include)
target_link_libraries(nlang_parser nlang_common)