cmake_minimum_required(VERSION 3.10)
project(nlang_vm)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED TRUE)

set(NLANG_VM_SOURCES
        vm.cpp)

set(NLANG_VM_HEADERS
        vm.hpp)

add_library(nlang_vm ${NLANG_VM_HEADERS} ${NLANG_VM_SOURCES})
target_include_directories(nlang_vm PUBLIC .)
target_link_libraries(nlang_vm nlang_common)