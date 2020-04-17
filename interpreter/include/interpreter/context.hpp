#pragma once

#include "handle.hpp"
#include "value.hpp"
#include "heap.hpp"

#include <stdexcept>
#include <vector>

namespace nlang {

struct ContextItemDescriptor {
    uint32_t context_index;
    uint32_t item_index;
};

class Context : public HeapValue {
public:
    friend struct ContextClass;

    Context() = default;
    virtual ~Context() = default;

    void SetItem(ContextItemDescriptor descriptor, Handle<Value> item) {
        if (!descriptor.context_index) {
            auto& i = items[descriptor.item_index];
            if (!item) {
                throw std::runtime_error("usage before definition");
            }
            i = item;
            return;
        }
        --descriptor.context_index;
        parent.As<Context>()->SetItem(descriptor, item);
    }

    Handle<Value> GetItem(ContextItemDescriptor descriptor) const {
        if (!descriptor.context_index) {
            auto& item = items[descriptor.item_index];
            if (!item) {
                throw std::runtime_error("usage before definition");
            }
            return item;
        }
        --descriptor.context_index;
        return parent.As<Context>()->GetItem(descriptor);
    }

    void DefineItem(ContextItemDescriptor descriptor) {
        if (!descriptor.context_index) {
            items[descriptor.item_index] = Null::New();
            return;
        }
        --descriptor.context_index;
        parent.As<Context>()->DefineItem(descriptor);
    }

    static Handle<Context> New(Heap* heap) {
        return heap->Store(new Context).As<Context>();
    }

private:
    Handle<HeapValue> parent;
    std::vector<Handle<Value>> items;
};

struct ContextClass : public HeapValue {
    size_t items_count = 0;
    std::vector<Handle<HeapValue>> functions;

    Handle<Context> Instantiate(Heap* heap, Handle<Context> parent_context);

    static Handle<ContextClass> New(Heap* heap, size_t items_count = 0, std::vector<Handle<HeapValue>> functions = {}) {
        auto handle = heap->Store(new ContextClass).As<ContextClass>();
        handle->items_count = items_count;
        handle->functions = std::move(functions);
        return handle;
    }
};

}