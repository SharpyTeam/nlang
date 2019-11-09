cmake_minimum_required(VERSION 3.12)
project(nlang_core)

set(CMAKE_CXX_STANDARD 17)

set(NLANG_CORE_SOURCES
        main.cpp)

set(NLANG_CORE_HEADERS
        main.hpp)

configure_file(version.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp")

add_executable(nlang ${NLANG_CORE_HEADERS} ${NLANG_CORE_SOURCES})
target_include_directories(nlang PUBLIC .)
target_link_libraries(nlang nlang_parser)
target_link_libraries(nlang nlang_compiler)
target_link_libraries(nlang nlang_vm)