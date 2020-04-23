set(NLANG_INTERPRETER_SOURCES
        src/stub.cpp
        src/function.cpp
        src/context.cpp
)

set(NLANG_INTERPRETER_HEADERS
        include/interpreter/interpreter.hpp
        include/interpreter/class.hpp
        include/interpreter/function.hpp
        include/interpreter/object.hpp
        include/interpreter/context.hpp
        include/interpreter/bytecode_executor.hpp
        include/interpreter/native_function.hpp
        include/interpreter/bytecode_function.hpp
)

add_subdirectory(ut)

add_library(nlang_interpreter ${NLANG_INTERPRETER_HEADERS} ${NLANG_INTERPRETER_SOURCES})
target_include_directories(nlang_interpreter PUBLIC include)
target_link_libraries(nlang_interpreter nlang_common)
target_link_libraries(nlang_interpreter nlang_parser)
target_link_libraries(nlang_interpreter Threads::Threads)