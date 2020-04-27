#pragma once

#include <interpreter/function.hpp>
#include <interpreter/context.hpp>
#include <interpreter/value.hpp>
#include <interpreter/handle.hpp>
#include <interpreter/heap.hpp>

#include <compiler/bytecode.hpp>

#include <cstdint>
#include <thread>
#include <cstdlib>

namespace nlang {

struct StackFrame {
    Handle<Context> context;
    Handle<Function> function;
    Handle<Value>* arguments = nullptr;
    Handle<Value>* registers = nullptr;
    bytecode::Instruction* ip = nullptr;

    StackFrame* next = nullptr;
    StackFrame* prev = nullptr;

    StackFrame(
            StackFrame* prev,
            Handle<Context> context,
            Handle<Function> function)
            : context(context)
            , function(function)
            , arguments(reinterpret_cast<Handle<Value>*>(this + 1))
            , registers(arguments + function->GetArgumentsCount())
            , next(reinterpret_cast<StackFrame*>(registers + function->GetRegistersCount()))
            , prev(prev)
    {
        for (Handle<Value>* current = arguments; current < static_cast<void*>(next); ++current) {
            new (current) Handle<Value>;
        }
    }
};

}