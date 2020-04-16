#pragma once

#include <cstdint>

namespace nlang {

enum class Opcode : uint8_t {
    RegToAcc,
    AccToReg,
    Add,
    Sub,
    Mul,
    Div,
    Ret,
    Call,
};

struct Instruction {
    Opcode opcode;
    union {
        int32_t register_index;
        struct {
            int32_t first;
            uint32_t count;
        } registers_range;
    } operand;
};

}