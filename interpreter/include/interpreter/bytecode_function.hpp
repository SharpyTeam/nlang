#pragma once

#include <interpreter/function.hpp>
#include <interpreter/heap.hpp>

#include <compiler/bytecode.hpp>

#include <vector>

namespace nlang {

class Thread;

/**
 * Represents a bytecode function.
 * Bytecode functions are language functions - functions that were created from source code.
 */
class BytecodeFunction : public Function {
public:
    /**
     * Returns count of registers, used by the function
     * @return Register count
     */
    int32_t GetRegistersCount() const override {
        return bytecode_chunk.registers_count;
    }

    /**
     * Returns count of arguments, used by the function
     * @return Argument count
     */
    int32_t GetArgumentsCount() const override {
        return bytecode_chunk.arguments_count;
    }

    /**
     * Invokes the function bytecode using BytecodeExecutor and given thread
     * @param thread The thread to execute in
     * @param args_count Arguments count
     * @param args Arguments
     */
    void DoInvoke(Thread* thread, int32_t args_count, const Handle<Value>* args) override;

    /**
     * Creates a bytecode function from given heap and bytecode chunk
     * @param heap The heap
     * @param bytecode_chunk The bytecode of the function
     * @return Handle to created function
     */
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