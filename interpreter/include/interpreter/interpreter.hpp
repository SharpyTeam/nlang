#pragma once

#include "value.hpp"
#include "handle.hpp"

#include <vector>
#include <cstdint>

namespace nlang {

class Compiler {

};


class Environment {
public:


};

class Thread {
public:


private:
    struct StackEntry {
        union {
            Handle<Value> handle;
            Handle<HeapValue> heap_handle;
            void* raw_pointer;
        };
    };

    Handle<Value>   accumulator;
    uint8_t*        instruction_pointer;
    StackEntry*     stack_pointer;


    Thread() {
        call_stack.reserve(8 * 1024 * 1024);
    }

    std::vector<StackEntry> call_stack;
};

}
