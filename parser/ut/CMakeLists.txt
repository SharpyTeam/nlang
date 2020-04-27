set(NLANG_PARSER_TESTS_SOURCES
        main.cpp
        scanner.cpp
        parser.cpp
)

add_executable(nlang_parser_tests ${NLANG_PARSER_TESTS_SOURCES})
target_link_libraries(nlang_parser_tests nlang_parser Catch2::Catch2)

catch_discover_tests(nlang_parser_tests)
