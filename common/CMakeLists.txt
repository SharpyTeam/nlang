set(NLANG_COMMON_SOURCES
        src/stub.cpp)

set(NLANG_COMMON_HEADERS
        include/common/token.hpp
        include/common/ast.hpp
        include/common/error_reporter.hpp)

add_library(nlang_common ${NLANG_COMMON_HEADERS} ${NLANG_COMMON_SOURCES})
target_include_directories(nlang_common PUBLIC include)
target_link_libraries(nlang_common nlang_utils)