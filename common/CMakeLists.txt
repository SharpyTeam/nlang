set(NLANG_COMMON_SOURCES
        src/stub.cpp
)

set(NLANG_COMMON_HEADERS
        include/common/ast.hpp
        include/common/error_reporter.hpp
        include/common/token.hpp
)

add_library(nlang_common STATIC ${NLANG_COMMON_HEADERS} ${NLANG_COMMON_SOURCES})
target_include_directories(nlang_common PUBLIC include)
target_link_libraries(nlang_common PUBLIC nlang_utils)