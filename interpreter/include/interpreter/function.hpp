#pragma once

#include "value.hpp"
#include "handle.hpp"
#include "context.hpp"
#include "heap.hpp"

#include <cstdint>
#include <vector>
#include <functional>

namespace nlang {

class Thread;

class Function : public HeapValue {
public:
    Function(Handle<ContextClass> context_class)
        : context_class(context_class)
    {}
    virtual ~Function() override = default;

    virtual size_t GetRegistersCount() const = 0;
    virtual size_t GetRegisterArgumentsCount() const = 0;

    virtual void DoInvoke(Thread* thread, size_t args_count, const Handle<Value>* args) = 0;

    static void Invoke(Thread* thread, Handle<Context> parent_context, Handle<Function> function, size_t args_count, const Handle<Value>* args);

private:
    Handle<ContextClass> context_class;
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
