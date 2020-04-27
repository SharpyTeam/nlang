#pragma once

#include <interpreter/function.hpp>
#include <interpreter/heap.hpp>

#include <compiler/bytecode.hpp>

#include <vector>

namespace nlang {

class Thread;

class BytecodeFunction : public Function {
public:
    int32_t GetRegistersCount() const override {
        return bytecode_chunk.registers_count;
    }

    int32_t GetArgumentsCount() const override {
        return bytecode_chunk.arguments_count;
    }

    void DoInvoke(Thread* thread, int32_t args_count, const Handle<Value>* args) override;

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