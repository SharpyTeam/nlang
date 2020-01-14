cmake_minimum_required(VERSION 3.10)
project(nlang_library)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

set(NLANG_LIBRARY_SOURCES
        stub.cpp)

set(NLANG_LIBRARY_HEADERS
        thread/thread_pool.hpp)

add_library(nlang_library ${NLANG_LIBRARY_HEADERS} ${NLANG_LIBRARY_SOURCES})
target_include_directories(nlang_library PUBLIC .)

target_link_libraries(nlang_library Threads::Threads)