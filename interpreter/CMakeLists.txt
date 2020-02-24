cmake_minimum_required(VERSION 3.10)
project(nlang_interpreter)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

set(NLANG_INTERPRETER_SOURCES
        src/stub.cpp
        src/function.cpp)

set(NLANG_INTERPRETER_HEADERS
        include/interpreter/ast_interpreter.hpp
        include/interpreter/heap.hpp
        include/interpreter/value.hpp
        include/interpreter/handle.hpp
        include/interpreter/interpreter.hpp
        include/interpreter/class.hpp
        include/interpreter/object.hpp
        include/interpreter/function.hpp)

add_library(nlang_interpreter ${NLANG_INTERPRETER_HEADERS} ${NLANG_INTERPRETER_SOURCES})
target_include_directories(nlang_interpreter PUBLIC include)
target_link_libraries(nlang_interpreter nlang_common)
target_link_libraries(nlang_interpreter nlang_parser)
target_link_libraries(nlang_interpreter Threads::Threads)