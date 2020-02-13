#pragma once

#include <cstdint>

namespace nlang {

class BytecodeEntity {};

class Instruction : public BytecodeEntity {
public:
    const uint8_t opcode;

    explicit Instruction(uint8_t opcode) : opcode(opcode) {}
};

class Label {};

}