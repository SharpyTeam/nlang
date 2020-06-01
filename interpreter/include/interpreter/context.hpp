#pragma once

#include <interpreter/handle.hpp>
#include <interpreter/objects/primitives.hpp>
#include <interpreter/heap.hpp>
#include <interpreter/objects/string.hpp>

#include <compiler/bytecode.hpp>

#include <stdexcept>
#include <vector>

namespace nlang {
/**
 * Represents the execution context
 */
class Context : public HeapValue {
public:
    Context() = default;
    virtual ~Context() = default;

    void Store(bytecode::ContextDescriptor descriptor, Handle<Value> value) {
        Context* context = this;
        while (descriptor.depth--) {
            context = &*context->parent;
        }
        if (!context->values[descriptor.index]) {
            throw;
        }
        context->values[descriptor.index] = value;
    }

    Handle<Value> Load(bytecode::ContextDescriptor descriptor) {
        Context* context = this;
        while (descriptor.depth--) {
            context = &*context->parent;
        }
        if (!context->values[descriptor.index]) {
            throw;
        }
        return context->values[descriptor.index];
    }

    void Declare(bytecode::ContextDescriptor descriptor) {
        Context* context = this;
        while (descriptor.depth--) {
            context = &*context->parent;
        }
        if (context->values[descriptor.index]) {
            throw;
        }
        context->values[descriptor.index] = Null::New();
    }

    Handle<Context> GetParent() const {
        return parent;
    }

    virtual void ForEachReference(std::function<void(Handle<Value>)> handler) override {
        handler(parent);
        std::for_each(values.begin(), values.end(), handler);
    }

    static Handle<Context> New(Heap* heap, Handle<Context> parent, int32_t size) {
        return heap->Store(new Context(parent, size)).As<Context>();
    }

private:
    Context(Handle<Context> parent, int32_t size)
        : parent(parent)
        , values(size)
    {}

    Handle<Context> parent;
    std::vector<Handle<Value>> values;
};


}