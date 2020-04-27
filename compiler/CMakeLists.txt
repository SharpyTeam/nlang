set(NLANG_COMPILER_SOURCES
        src/stub.cpp)

set(NLANG_COMPILER_HEADERS
        include/compiler/bytecode.hpp
        include/compiler/compiler.hpp
        include/compiler/registers_shape.hpp
        include/compiler/scope.hpp
        include/compiler/semantic_analyser.hpp
)

set(NLANG_COMPILER_PROTO_SOURCES
        proto/class_def.proto)

compile_proto(NLANG_COMPILER_PROTO_SOURCES NLANG_COMPILER_SOURCES NLANG_COMPILER_HEADERS)

add_library(nlang_compiler STATIC ${NLANG_COMPILER_HEADERS} ${NLANG_COMPILER_SOURCES})
target_include_directories(nlang_compiler PUBLIC include)
target_link_libraries(nlang_compiler PUBLIC nlang_interpreter nlang_common nlang_utils protobuf::libprotobuf)