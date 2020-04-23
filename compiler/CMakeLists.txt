set(NLANG_COMPILER_SOURCES
        src/stub.cpp)

set(NLANG_COMPILER_HEADERS
        include/compiler/semantic_analyser.hpp
        include/compiler/scope.hpp
        include/compiler/compiler.hpp
)

set(NLANG_COMPILER_PROTO_SOURCES
        proto/class_def.proto)

compile_proto(NLANG_COMPILER_PROTO_SOURCES NLANG_COMPILER_SOURCES NLANG_COMPILER_HEADERS)

add_library(nlang_compiler ${NLANG_COMPILER_HEADERS} ${NLANG_COMPILER_SOURCES})
target_include_directories(nlang_compiler PUBLIC include)
target_link_libraries(nlang_compiler nlang_common nlang_utils nlang_interpreter protobuf::libprotobuf)