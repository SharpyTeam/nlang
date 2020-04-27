set(NLANG_INTERPRETER_SOURCES
        src/function.cpp
)

set(NLANG_INTERPRETER_HEADERS
        include/interpreter/objects/primitives.hpp
        include/interpreter/objects/string.hpp

        include/interpreter/bytecode_executor.hpp
        include/interpreter/bytecode_function.hpp
        include/interpreter/class.hpp
        include/interpreter/context.hpp
        include/interpreter/function.hpp
        include/interpreter/gc.hpp
        include/interpreter/handle.hpp
        include/interpreter/heap.hpp
        include/interpreter/native_function.hpp
        include/interpreter/object.hpp
        include/interpreter/stack_frame.hpp
        include/interpreter/thread.hpp
        include/interpreter/value.hpp
)

add_subdirectory(ut)

add_library(nlang_interpreter ${NLANG_INTERPRETER_HEADERS} ${NLANG_INTERPRETER_SOURCES})
target_include_directories(nlang_interpreter PUBLIC include)
target_link_libraries(nlang_interpreter PUBLIC nlang_compiler nlang_common nlang_utils Threads::Threads)