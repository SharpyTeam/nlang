#pragma once

#include "function.hpp"
#include "bytecode_executor.hpp"

#include <common/bytecode.hpp>

#include <vector>

namespace nlang {

class BytecodeFunction : public Function {
public:
    size_t GetRegistersCount() const override {
        return registers_count;
    }

    size_t GetRegisterArgumentsCount() const override {
        return register_arguments_count;
    }

    void DoInvoke(Thread* thread, size_t args_count, const Handle<Value>* args) override {
        thread->ip = instructions.data();
        for (size_t i = 0; i < args_count; ++i) {
            thread->sp->arguments[i] = args[i];
        }
        if (!thread->sp->prev || !thread->sp->prev->ip) {
            BytecodeExecutor::Execute(thread);
        }
    }

    static Handle<BytecodeFunction> New(Heap* heap, size_t registers_count, size_t register_arguments_count, std::vector<Instruction> instructions) {
        return heap->Store(new BytecodeFunction(registers_count, register_arguments_count, std::move(instructions))).As<BytecodeFunction>();
    }

private:
    explicit BytecodeFunction(size_t registers_count, size_t register_arguments_count, std::vector<Instruction>&& instructions)
        : registers_count(registers_count)
        , register_arguments_count(register_arguments_count)
        , instructions(std::move(instructions))
    {}

    size_t registers_count;
    size_t register_arguments_count;
    std::vector<Instruction> instructions;
};

}