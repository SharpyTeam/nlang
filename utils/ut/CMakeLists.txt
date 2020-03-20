set(NLANG_UTILS_TESTS_SOURCES
        main.cpp
        forward_list_view.cpp
        page_allocation.cpp
        slot_storage.cpp
        nan_boxed_primitive.cpp)

add_executable(nlang_utils_test ${NLANG_UTILS_TESTS_SOURCES})
target_link_libraries(nlang_utils_test nlang_utils Catch2::Catch2)

catch_discover_tests(nlang_utils_test)
