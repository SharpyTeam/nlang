set(NLANG_INTERPRETER_SOURCES
        src/stub.cpp)

set(NLANG_INTERPRETER_HEADERS
        include/interpreter/interpreter.hpp
        include/interpreter/class.hpp
        include/interpreter/function.hpp
        include/interpreter/handle.hpp
        include/interpreter/heap.hpp
        include/interpreter/object.hpp
        include/interpreter/string.hpp
        include/interpreter/value.hpp)

add_subdirectory(ut)

add_library(nlang_interpreter ${NLANG_INTERPRETER_HEADERS} ${NLANG_INTERPRETER_SOURCES})
target_include_directories(nlang_interpreter PUBLIC include)
target_link_libraries(nlang_interpreter nlang_common)
target_link_libraries(nlang_interpreter nlang_parser)
target_link_libraries(nlang_interpreter Threads::Threads)