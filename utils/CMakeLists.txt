cmake_minimum_required(VERSION 3.10)
project(nlang_utils)

set(CMAKE_CXX_STANDARD 17)

set(NLANG_UTILS_SOURCES
        src/backtrace.cpp)

set(NLANG_UTILS_HEADERS
        include/utils/defs.hpp
        include/utils/nan_boxed_primitive.hpp
        include/utils/traits.hpp
        include/utils/backtrace.hpp
        include/utils/thread_pool.hpp)

add_library(nlang_utils ${NLANG_UTILS_HEADERS} ${NLANG_UTILS_SOURCES})
target_include_directories(nlang_utils PUBLIC include)

add_subdirectory(deps/intrusive_ptr)
target_link_libraries(nlang_utils intrusive_ptr)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_subdirectory(deps/backward)
    target_link_libraries(nlang_utils backward)
endif ()
