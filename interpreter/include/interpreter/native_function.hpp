#pragma once

#include "function.hpp"
#include "interpreter.hpp"
#include "heap.hpp"
#include "handle.hpp"
#include "value.hpp"

#include <functional>

namespace nlang {

class NativeFunction : public Function {
public:
    size_t GetRegistersCount() const override {
        return 0;
    }

    size_t GetRegisterArgumentsCount() const override {
        return 0;
    }

    void DoInvoke(Thread* thread, size_t args_count, const Handle<Value>* args) override {
        thread->ip = nullptr;
        thread->acc = function(thread, thread->sp->context, args_count, args);
        thread->PopFrame();
    }

    static Handle<NativeFunction> New(Heap* heap, std::function<Handle<Value>(Thread*, Handle<Context>, size_t, const Handle<Value>*)> function) {
        return heap->Store(new NativeFunction(std::move(function))).As<NativeFunction>();
    }

private:
    explicit NativeFunction(std::function<Handle<Value>(Thread*, Handle<Context>, size_t, const Handle<Value>*)>&& function)
        : function(std::move(function))
    {}

    std::function<Handle<Value>(Thread*, Handle<Context>, size_t, const Handle<Value>*)> function;
};

}