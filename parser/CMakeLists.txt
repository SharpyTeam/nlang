set(NLANG_PARSER_SOURCES
        src/scanner.cpp
        src/token_stream.cpp
)

set(NLANG_PARSER_HEADERS
        include/parser/parser.hpp
        include/parser/scanner.hpp
        include/parser/stream_cache.hpp
        include/parser/token_stream.hpp
)

add_subdirectory(ut)

add_library(nlang_parser ${NLANG_PARSER_HEADERS} ${NLANG_PARSER_SOURCES})
target_include_directories(nlang_parser PUBLIC include)
target_link_libraries(nlang_parser PUBLIC nlang_common nlang_utils)