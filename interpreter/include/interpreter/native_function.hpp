#pragma once

#include "function.hpp"
#include "thread.hpp"
#include "common/heap/heap.hpp"
#include "common/handles/handle.hpp"
#include "common/values/value.hpp"

#include <functional>

namespace nlang {

class NativeFunction : public Function {
public:
    size_t GetRegistersCount() const override {
        return 0;
    }

    size_t GetArgumentsCount() const override {
        return 0;
    }

    void DoInvoke(Thread* thread, size_t args_count, const Handle<Value>* args) override {
        thread->acc = function(thread, thread->sp->context, args_count, args);
        thread->PopFrame();
    }

    static Handle<NativeFunction> New(Heap* heap, std::function<Handle<Value>(Thread*, Handle<Context>, size_t, const Handle<Value>*)> function) {
        return heap->Store(new NativeFunction(ContextClass::New(heap), std::move(function))).As<NativeFunction>();
    }

    // No references => does nothing.
    void ForEachReference(std::function<void(Handle<Value>)> handler) override {}

private:
    explicit NativeFunction(Handle<ContextClass> context_class, std::function<Handle<Value>(Thread*, Handle<Context>, size_t, const Handle<Value>*)>&& function)
        : Function(context_class)
        , function(std::move(function))
    {}

    std::function<Handle<Value>(Thread*, Handle<Context>, size_t, const Handle<Value>*)> function;
};

}