#pragma once

#include "handle.hpp"
#include "value.hpp"
#include "heap.hpp"

namespace nlang {

struct Context : public HeapValue {
    Handle<HeapValue> parent;

    static Handle<Context> New(Heap* heap) {
        return heap->Store(new Context).As<Context>();
    }
};

}