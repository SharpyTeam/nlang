cmake_minimum_required(VERSION 3.10)

project(nlang VERSION 0.0.0 LANGUAGES CXX)


# C++ features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Suppress CMake unused variable warning
set(dummy_var "${CMAKE_C_COMPILER}")


# Build configuration
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (${IS_BIG_ENDIAN})
    message(FATAL_ERROR "Big-Endian architectures are not supported.")
endif ()

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif ()


# Packages
list(APPEND CMAKE_PREFIX_PATH deps/backward-cpp)
find_package(Backward)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Git)
find_package(Catch2)


# Compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # MATCHES cause on macOS it is AppleClang now
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")

    if (ENABLE_SANITIZERS)
        message(STATUS "Enabling asan & ubsan sanitizers")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined,address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined,address")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # remove default warning level from CMAKE_CXX_FLAGS_INIT
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
    # define macro to suppress codecvt and wstring_convert deprecation warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING")

    if (ENABLE_SANITIZERS)
        message(WARNING "Build with sanitizers is not supported on MSVC yet")
    endif ()
endif ()


# Testing stuff
include(CTest)
include(Catch)


# Protobuf integration
function (compile_proto SRCS OUT_SOURCES OUT_HEADERS)
    foreach (SRC ${${SRCS}})
        protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${SRC})
    endforeach ()
    list(APPEND ${OUT_SOURCES} "${PROTO_SOURCES}")
    list(APPEND ${OUT_HEADERS} "${PROTO_HEADERS}")
    set(${OUT_SOURCES} ${${OUT_SOURCES}} PARENT_SCOPE)
    set(${OUT_HEADERS} ${${OUT_HEADERS}} PARENT_SCOPE)
endfunction ()


# Version definitions
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

configure_file(version.hpp.in "${CMAKE_BINARY_DIR}/version/version.hpp")


# Subprojects
add_subdirectory(utils)
add_subdirectory(common)
add_subdirectory(parser)
add_subdirectory(compiler)
add_subdirectory(interpreter)

if (ENABLE_JNI_TOOLS)
    message(STATUS "Enabling JNI tools builds")
    add_subdirectory(ide/jni)
endif ()


# nlang executable
set(NLANG_SOURCES
        main.cpp)

set(NLANG_HEADERS
        )

add_executable(nlang ${NLANG_HEADERS} ${NLANG_SOURCES})
target_link_libraries(nlang nlang_utils)
target_link_libraries(nlang nlang_common)
target_link_libraries(nlang nlang_parser)
target_link_libraries(nlang nlang_compiler)
target_link_libraries(nlang nlang_interpreter)
