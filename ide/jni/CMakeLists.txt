cmake_minimum_required(VERSION 3.10)

project("nlang jni tools")

set(CMAKE_CXX_STANDARD 17)

find_package(JNI REQUIRED)

set(NLANG_JNI_SOURCES
        main.cpp)
set(NLANG_JNI_HEADERS
        )

add_library(nlang_jni SHARED ${NLANG_JNI_HEADERS} ${NLANG_JNI_SOURCES})
target_include_directories(nlang_jni PUBLIC ${JNI_INCLUDE_DIRS})
target_link_libraries(nlang_jni PUBLIC nlang_parser)