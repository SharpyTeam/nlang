cmake_minimum_required(VERSION 3.10)

project(nlang VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # MATCHES cause on macOS it is AppleClang now
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /UTF-8")
endif()

find_package(Git)
if (Git_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            RESULT_VARIABLE GIT_REVISION_FETCH_RESULT
            OUTPUT_VARIABLE GIT_REVISION)
    string(REGEX REPLACE "\n$" "" GIT_REVISION "${GIT_REVISION}")
    message("Git revision: ${GIT_REVISION}")
else ()
    message("Can't find Git. Falling back to 'dev' revision in version.hpp")
    set(GIT_REVISION "dev")
endif ()

find_package(Threads REQUIRED)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (${IS_BIG_ENDIAN})
    message(FATAL_ERROR "Big-Endian architectures are not supported.")
endif ()

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/Catch2)

add_subdirectory(utils)
add_subdirectory(common)
add_subdirectory(parser)
add_subdirectory(interpreter)

add_subdirectory(tests)

set(NLANG_CORE_SOURCES
        main.cpp)

set(NLANG_CORE_HEADERS
        )

configure_file(version.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp")

add_executable(nlang ${NLANG_CORE_HEADERS} ${NLANG_CORE_SOURCES})
target_link_libraries(nlang nlang_common)
target_link_libraries(nlang nlang_parser)
target_link_libraries(nlang nlang_interpreter)
target_link_libraries(nlang Threads::Threads)

if (ENABLE_JNI_TOOLS)
    add_subdirectory(ide/jni)
endif ()
