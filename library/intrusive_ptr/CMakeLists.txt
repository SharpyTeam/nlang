cmake_minimum_required(VERSION 3.10)
project(IntrusivePtr)

set(CMAKE_CXX_STANDARD 17)

add_library(intrusive_ptr INTERFACE)
target_include_directories(intrusive_ptr INTERFACE src)