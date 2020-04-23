set(NLANG_COMMON_SOURCES
        src/stub.cpp
)

set(NLANG_COMMON_HEADERS
        include/common/environment.hpp
        include/common/token.hpp
        include/common/error_reporter.hpp
        include/common/bytecode.hpp

        include/common/ast/ast.hpp

        include/common/handles/handle.hpp

        include/common/values/value.hpp
        include/common/values/primitives.hpp

        include/common/heap/heap.hpp

        include/common/objects/string.hpp
)

add_library(nlang_common ${NLANG_COMMON_HEADERS} ${NLANG_COMMON_SOURCES})
target_include_directories(nlang_common PUBLIC include)
target_link_libraries(nlang_common nlang_utils ICU::i18n ICU::uc)