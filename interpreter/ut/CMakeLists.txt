set(NLANG_INTERPRETER_TESTS_SOURCES
        main.cpp
        thread_spawn_and_execution.cpp
        class_and_object_creation.cpp
        internal_string.cpp)

add_executable(nlang_interpreter_tests ${NLANG_INTERPRETER_TESTS_SOURCES})
target_link_libraries(nlang_interpreter_tests nlang_interpreter Catch2::Catch2)

catch_discover_tests(nlang_interpreter_tests)
