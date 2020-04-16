#pragma once

#include "value.hpp"
#include "handle.hpp"
#include "function.hpp"
#include "heap.hpp"
#include "context.hpp"

#include <common/bytecode.hpp>

#include <vector>
#include <cstdint>
#include <thread>
#include <cstdlib>

namespace nlang {

struct StackFrame {
    Handle<Context> context;
    Handle<Function> function;
    Handle<Value>* arguments = nullptr;
    Handle<Value>* registers = nullptr;
    Instruction* ip = nullptr;

    StackFrame* next = nullptr;
    StackFrame* prev = nullptr;

    StackFrame(
            StackFrame* prev,
            Handle<Context> context,
            Handle<Function> function)
        : context(context)
        , function(function)
        , arguments(reinterpret_cast<Handle<Value>*>(this + 1))
        , registers(arguments + function->GetRegisterArgumentsCount())
        , next(reinterpret_cast<StackFrame*>(registers + function->GetRegistersCount()))
        , prev(prev)
    {
        for (Handle<Value>* current = arguments; current < static_cast<void*>(next); ++current) {
            new (current) Handle<Value>;
        }
    }
};

class Thread {
public:
    Thread(Heap* heap, Handle<Closure> closure, size_t args_count, const Handle<Value>* args) noexcept
        : heap(heap)
        , mem(aligned_alloc(alignof(StackFrame), 8 * 1024 * 1024))
    {
        thread = std::thread(&Thread::Run, this, closure, std::vector<Handle<Value>>(args, args + args_count));
    }

    ~Thread() noexcept {
        free(mem);
    }

    Handle<Value> Join() {
        thread.join();
        return acc;
    }

public:
    void PushFrame(Handle<Context> context, Handle<Function> function) {
        if (sp) {
            sp->ip = ip;
        }
        sp = new (sp ? sp->next : static_cast<StackFrame*>(mem)) StackFrame(sp, context, function);
    }

    void PopFrame() {
        sp = sp->prev;
        if (sp) {
            ip = sp->ip;
        }
    }

    void Run(Handle<Closure> closure, std::vector<Handle<Value>>&& args) {
        closure->Invoke(this, args.size(), args.data());
    }

public:
    Heap* heap;
    Instruction* ip = nullptr;
    StackFrame* sp = nullptr;
    Handle<Value> acc;

private:
    std::thread thread;
    void* mem = nullptr;
};

}
