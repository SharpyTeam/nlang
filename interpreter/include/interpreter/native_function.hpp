#pragma once

#include <interpreter/function.hpp>
#include <interpreter/thread.hpp>

#include <interpreter/heap.hpp>
#include <interpreter/handle.hpp>
#include <interpreter/value.hpp>

#include <functional>

namespace nlang {

class NativeFunction : public Function {
public:
    int32_t GetRegistersCount() const override {
        return 0;
    }

    int32_t GetArgumentsCount() const override {
        return 0;
    }

    void DoInvoke(Thread* thread, int32_t args_count, const Handle<Value>* args) override {
        thread->acc = function(thread, thread->sp->context, args_count, args);
        thread->PopFrame();
    }

    static Handle<NativeFunction> New(Heap* heap, std::function<Handle<Value>(Thread*, Handle<Context>, int32_t, const Handle<Value>*)> function) {
        return heap->Store(new NativeFunction(std::move(function))).As<NativeFunction>();
    }

    // No references => does nothing.
    void ForEachReference(std::function<void(Handle<Value>)> handler) override {}

private:
    explicit NativeFunction(std::function<Handle<Value>(Thread*, Handle<Context>, int32_t, const Handle<Value>*)>&& function)
        : function(std::move(function))
    {}

    std::function<Handle<Value>(Thread*, Handle<Context>, int32_t, const Handle<Value>*)> function;
};

}