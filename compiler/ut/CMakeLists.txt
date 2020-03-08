set(NLANG_COMPILER_TESTS_SOURCES
        main.cpp)

add_executable(nlang_compiler_tests ${NLANG_COMPILER_TESTS_SOURCES})
target_link_libraries(nlang_compiler_tests nlang_compiler Catch2::Catch2)

catch_discover_tests(nlang_compiler_tests)
