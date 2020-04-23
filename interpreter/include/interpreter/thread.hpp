#pragma once


#include "function.hpp"
#include "context.hpp"
#include "stack_frame.hpp"

#include <common/bytecode.hpp>
#include <common/values/value.hpp>
#include <common/handles/handle.hpp>
#include <common/heap/heap.hpp>
#include <utils/platform_dependent.hpp>

#include <vector>
#include <cstdint>
#include <thread>
#include <cstdlib>

namespace nlang {

class Thread {
public:
    Thread(Heap* heap, Handle<Closure> closure, size_t args_count, const Handle<Value>* args) noexcept
            : heap(heap)
            , mem(do_aligned_alloc(alignof(StackFrame), 8 * 1024 * 1024))
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
        ip = nullptr;
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
    bytecode::Instruction* ip = nullptr;
    StackFrame* sp = nullptr;
    Handle<Value> acc;

private:
    std::thread thread;
    void* mem = nullptr;
};

}