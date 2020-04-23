#pragma once

#include "common/values/value.hpp"
#include "common/handles/handle.hpp"
#include "context.hpp"
#include "common/heap/heap.hpp"

#include <cstdint>
#include <vector>
#include <functional>

namespace nlang {

class Thread;

class Function : public HeapValue {
public:
    Function() = default;
    virtual ~Function() override = default;

    virtual size_t GetRegistersCount() const = 0;
    virtual size_t GetArgumentsCount() const = 0;

    virtual void DoInvoke(Thread* thread, size_t args_count, const Handle<Value>* args) = 0;

    static void Invoke(Thread* thread, Handle<Context> context, Handle<Function> function, size_t args_count, const Handle<Value>* args);

    virtual void ForEachReference(std::function<void(Handle<Value>)> handler) override = 0;
};


class Closure : public HeapValue {
public:
    Handle<Value> Call(Thread* thread, size_t args_count, const Handle<Value>* args);

    void Invoke(Thread* thread, size_t args_count, const Handle<Value>* args) {
        Function::Invoke(thread, context, function, args_count, args);
    }

    static Handle<Closure> New(Heap* heap, Handle<Context> context, Handle<Function> function) {
        return heap->Store(new Closure(context, function)).As<Closure>();
    }

    static Handle<Closure> New(Heap* heap, Handle<Function> function) {
        return New(heap, Handle<Context>(), function);
    }

    void ForEachReference(std::function<void(Handle<Value>)> handler) {
        handler(context);
        handler(function);
    }

private:
    Closure(Handle<Context> context, Handle<Function> function)
        : context(context)
        , function(function)
    {}

public:
    Handle<Context> context;
    Handle<Function> function;
};

}
