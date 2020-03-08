set(NLANG_COMPILER_SOURCES
        src/stub.cpp)

set(NLANG_COMPILER_HEADERS
        )

set(NLANG_COMPILER_PROTO_SOURCES
        proto/class_def.proto)

compile_proto(NLANG_COMPILER_PROTO_SOURCES NLANG_COMPILER_SOURCES NLANG_COMPILER_HEADERS)

add_library(nlang_compiler ${NLANG_COMPILER_HEADERS} ${NLANG_COMPILER_SOURCES})
target_include_directories(nlang_compiler PUBLIC include)
target_include_directories(nlang_compiler PUBLIC ${CMAKE_BINARY_DIR})
target_link_libraries(nlang_compiler nlang_utils)
target_link_libraries(nlang_compiler protobuf::libprotobuf)