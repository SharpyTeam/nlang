#pragma once

#include "interpreter.hpp"

namespace nlang {

class BytecodeExecutor {
    void Execute(Thread* thread);
};

}