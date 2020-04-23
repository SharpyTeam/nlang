#pragma once

#include <common/handles/handle.hpp>
#include <common/values/primitives.hpp>
#include <common/heap/heap.hpp>
#include <common/objects/string.hpp>

#include <stdexcept>
#include <vector>

namespace nlang {

class Context : public HeapValue {
public:
    struct ContextDescriptor {
        uint32_t index;
        uint32_t depth;
    };

    Context() = default;
    virtual ~Context() = default;

    void Store(ContextDescriptor descriptor, Handle<Value> value) {
        Context* context = this;
        while (descriptor.depth--) {
            context = &*context->parent;
        }
        if (!context->values[descriptor.index]) {
            throw;
        }
        context->values[descriptor.index] = value;
    }

    Handle<Value> Load(ContextDescriptor descriptor) {
        Context* context = this;
        while (descriptor.depth--) {
            context = &*context->parent;
        }
        if (!context->values[descriptor.index]) {
            throw;
        }
        return context->values[descriptor.index];
    }

    void Declare(ContextDescriptor descriptor) {
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

    static Handle<Context> New(Heap* heap, Handle<Context> parent, size_t size) {
        return heap->Store(new Context(parent, size)).As<Context>();
    }

private:
    Context(Handle<Context> parent, size_t size)
        : parent(parent)
        , values(size)
    {}

    Handle<Context> parent;
    std::vector<Handle<Value>> values;
};


}