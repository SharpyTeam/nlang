project(nlang_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(NLANG_TESTS_SOURCES
        tests_main.cpp
        page_allocation.cpp
        internal_string.cpp
        thread_spawn_and_execution.cpp
        forward_list_view.cpp
        slot_storage.cpp
        class_and_object_creation.cpp)

include(CTest)
include(${CMAKE_SOURCE_DIR}/deps/Catch2/contrib/Catch.cmake)

add_executable(nlang_tests ${NLANG_TESTS_SOURCES})
target_link_libraries(nlang_tests Catch2::Catch2)
target_link_libraries(nlang_tests nlang_interpreter)

enable_testing()

catch_discover_tests(nlang_tests)
