#pragma once

#include <interpreter/function.hpp>
#include <interpreter/interpreter.hpp>
//#include "bytecode_executor.hpp"

#include <common/bytecode.hpp>

#include <vector>

namespace nlang {

class BytecodeFunction : public Function {
public:
    size_t GetRegistersCount() const override {
        return bytecode_chunk.registers_count;
    }

    size_t GetArgumentsCount() const override {
        return bytecode_chunk.arguments_count;
    }

    void DoInvoke(Thread* thread, size_t args_count, const Handle<Value>* args) override;

    static Handle<BytecodeFunction> New(Heap* heap, bytecode::BytecodeChunk&& bytecode_chunk) {
        return heap->Store(new BytecodeFunction(std::move(bytecode_chunk))).As<BytecodeFunction>();
    }

    void ForEachReference(std::function<void(Handle<Value>)> handler) override {

    }

private:
    explicit BytecodeFunction(bytecode::BytecodeChunk&& bytecode_chunk)
        : bytecode_chunk(bytecode_chunk)
    {}

public:
    bytecode::BytecodeChunk bytecode_chunk;
};

}