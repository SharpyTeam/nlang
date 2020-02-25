cmake_minimum_required(VERSION 3.10)

project(nlang VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # MATCHES cause on macOS it is AppleClang now
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # remove default warning level from CMAKE_CXX_FLAGS_INIT
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
endif ()

find_package(Protobuf REQUIRED)

function (compile_proto SRCS OUT_SOURCES OUT_HEADERS)
    foreach (SRC ${${SRCS}})
        protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${SRC})
    endforeach ()
    list(APPEND ${OUT_SOURCES} "${PROTO_SOURCES}")
    list(APPEND ${OUT_HEADERS} "${PROTO_HEADERS}")
    set(${OUT_SOURCES} ${${OUT_SOURCES}} PARENT_SCOPE)
    set(${OUT_HEADERS} ${${OUT_HEADERS}} PARENT_SCOPE)
endfunction ()

find_package(Git)
if (Git_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            RESULT_VARIABLE GIT_REVISION_FETCH_RESULT
            OUTPUT_VARIABLE GIT_REVISION)
    string(REGEX REPLACE "\n$" "" GIT_REVISION "${GIT_REVISION}")
    message(STATUS "Git revision: ${GIT_REVISION}")
else ()
    message(STATUS "Can't find Git. Falling back to 'dev' revision in version.hpp")
    set(GIT_REVISION "dev")
endif ()

if (ENABLE_SANITIZERS)
    message(STATUS "Enabling asan & ubsan sanitizers")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address")
endif ()

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
add_subdirectory(compiler)

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
target_link_libraries(nlang nlang_compiler)

if (ENABLE_JNI_TOOLS)
    message(STATUS "Enabling JNI tools builds")
    add_subdirectory(ide/jni)
endif ()
