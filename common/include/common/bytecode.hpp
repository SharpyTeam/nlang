#pragma once

#include <common/handles/handle.hpp>
#include <common/values/value.hpp>

#include <cstdint>
#include <vector>
#include <type_traits>
#include <unordered_map>

namespace nlang::bytecode {


using JumpLabel = int64_t;
using Label = int64_t;
using Offset = int64_t;
using Register = int32_t;
using ConstantIndex = int32_t;
using ImmediateInt32 = int32_t;
using ImmediateDouble = double;
using NoOperand = std::nullptr_t;


struct RegistersRange {
    Register first;
    uint32_t count;
};

struct ContextDescriptor {
    uint32_t index;
    uint32_t depth;
};


#define OPCODES                             \
                                            \
O(NoOperation,         NoOperand)              \
                                               \
O(LoadRegister,        Register)               \
O(StoreRegister,       Register)               \
                                               \
O(Add,                 Register)               \
O(Sub,                 Register)               \
O(Mul,                 Register)               \
O(Div,                 Register)               \
                                               \
O(DeclareContext,      ContextDescriptor)      \
O(LoadContext,         ContextDescriptor)      \
O(StoreContext,        ContextDescriptor)      \
                                               \
O(LoadConstant,        ConstantIndex)          \
                                               \
O(Call,                RegistersRange)         \
                                               \
O(Jump,                Offset)                 \
O(JumpIfTrue,          Offset)                 \
O(JumpIfFalse,         Offset)                 \
                                               \
O(CheckEqual,          Register)               \
O(CheckNotEqual,       Register)               \
O(CheckLess,           Register)               \
O(CheckGreater,        Register)               \
O(CheckLessOrEqual,    Register)               \
O(CheckGreaterOrEqual, Register)               \
O(CheckTypeEqual,      Register)               \
                                               \
O(PushContext,         ImmediateInt32)         \
                                               \
O(LoadNumber,          ImmediateDouble)        \
                                               \
O(PopContext,          NoOperand)              \
O(CreateClosure,       NoOperand)              \
O(Return,              NoOperand)              \
                                               \
O(LoadNull,            NoOperand)              \
O(LoadTrue,            NoOperand)              \
O(LoadFalse,           NoOperand)              \


enum class Opcode : uint8_t {
#define O(opcode, OperandType) opcode,
OPCODES
#undef O
};

template<Opcode opcode>
struct OpcodeTraits {};

#define O(op, OpType)                       \
template<>                                  \
struct OpcodeTraits<Opcode::op> {           \
    using OperandType = OpType;             \
};
OPCODES
#undef O



struct Instruction {
    Opcode opcode;
    union {
        Offset offset;
        Register reg;
        RegistersRange reg_range;
        ConstantIndex const_index;
        ContextDescriptor context_descriptor;
        ImmediateInt32 immediate_int32;
        ImmediateDouble immediate_double;
    };
};


struct BytecodeChunk {
    size_t arguments_count;
    size_t registers_count;
    std::vector<Instruction> bytecode;
    std::vector<Handle<Value>> constant_pool;
};


class BytecodeGenerator {
public:
    Label GetLabel() const {
        return chunk.bytecode.size();
    }

    Label EmitInstruction(const Instruction& instruction) {
        chunk.bytecode.push_back(instruction);
        return chunk.bytecode.size() - 1;
    }

    template<Opcode opcode>
    Label EmitInstruction() {
        Instruction instruction {};
        instruction.opcode = opcode;
        return EmitInstruction(instruction);
    }

    template<Opcode opcode>
    Label EmitInstruction(typename OpcodeTraits<opcode>::OperandType operand) {
        using Operand = typename OpcodeTraits<opcode>::OperandType;
        Instruction instruction {};
        instruction.opcode = opcode;
        if constexpr (std::is_same_v<Operand, Register>) {
            instruction.reg = operand;
        } else if constexpr (std::is_same_v<Operand, ContextDescriptor>) {
            instruction.context_descriptor = operand;
        } else if constexpr (std::is_same_v<Operand, RegistersRange>) {
            instruction.reg_range = operand;
        } else if constexpr (std::is_same_v<Operand, Offset>) {
            instruction.offset = operand;
        } else if constexpr (std::is_same_v<Operand, ConstantIndex>) {
            instruction.const_index = operand;
        } else if constexpr (std::is_same_v<Operand, ImmediateDouble>) {
            instruction.immediate_double = operand;
        } else if constexpr (std::is_same_v<Operand, ImmediateInt32>) {
            instruction.immediate_int32 = operand;
        }
        return EmitInstruction(instruction);
    }

    template<Opcode jump_opcode>
    JumpLabel EmitJump(Label to = 0) {
        static_assert(
                jump_opcode == Opcode::Jump ||
                jump_opcode == Opcode::JumpIfTrue ||
                jump_opcode == Opcode::JumpIfFalse);

        Instruction instruction {};
        instruction.opcode = jump_opcode;
        instruction.offset = to - chunk.bytecode.size();

        EmitInstruction(instruction);

        return chunk.bytecode.size() - 1;
    }

    void SetJumpToNextLabel(JumpLabel jump_label) {
        chunk.bytecode[jump_label].offset = chunk.bytecode.size() - jump_label;
    }

    void UpdateJump(JumpLabel jump_label, Label to) {
        chunk.bytecode[jump_label].offset = to - jump_label;
    }

    size_t StoreConstant(Handle<Value> constant) {
        chunk.constant_pool.emplace_back(constant);
        return chunk.constant_pool.size() - 1;
    }

    void SetRegistersCount(size_t count) {
        chunk.registers_count = count;
    }

    void SetArgumentsCount(size_t count) {
        chunk.arguments_count = count;
    }

    size_t GetLastEmmittedInstructionAddress() const {
        return chunk.bytecode.size() - 1;
    }

    BytecodeChunk Flush() {
        chunk.bytecode.shrink_to_fit();
        BytecodeChunk old_chunk;
        std::swap(chunk, old_chunk);
        return old_chunk;
    }

private:
    BytecodeChunk chunk;
};

class BytecodeDisassembler {
public:
    static std::string Disassemble(BytecodeChunk& bytecode_chunk) {
        std::string result = "arguments: " + std::to_string(bytecode_chunk.arguments_count) + " registers: " + std::to_string(bytecode_chunk.registers_count) + "\n";
        for (auto& i : bytecode_chunk.bytecode) {
            result += "\n" + names[i.opcode] + " " + printers[i.opcode](i);
        }
        return result;
    }

private:
    template<typename Operand>
    static std::function<std::string(Instruction)> GetPrinter() {
        if constexpr (std::is_same_v<Operand, Register>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.reg); };
        } else if constexpr (std::is_same_v<Operand, ContextDescriptor>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.context_descriptor.index) + " " + std::to_string(instruction.context_descriptor.depth); };
        } else if constexpr (std::is_same_v<Operand, RegistersRange>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.reg_range.first) + " " + std::to_string(instruction.reg_range.count); };
        } else if constexpr (std::is_same_v<Operand, Offset>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.offset); };
        } else if constexpr (std::is_same_v<Operand, ConstantIndex>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.const_index); };
        } else if constexpr (std::is_same_v<Operand, ImmediateDouble>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.immediate_double); };
        } else if constexpr (std::is_same_v<Operand, ImmediateInt32>) {
            return [](Instruction instruction) -> std::string { return std::to_string(instruction.immediate_int32); };
        } else if constexpr (std::is_same_v<Operand, NoOperand>) {
            return [](Instruction instruction) -> std::string { return ""; };
        }
        throw;
    }

    static inline std::unordered_map<Opcode, std::string> names {
#define O(op, OpType) { Opcode::op, #op },
        OPCODES
#undef O
    };

    static inline std::unordered_map<Opcode, std::function<std::string(Instruction)>> printers {
#define O(op, OpType) { Opcode::op, GetPrinter<typename OpcodeTraits<Opcode::op>::OperandType>() },
        OPCODES
#undef O
    };
};

#undef OPCODES

}