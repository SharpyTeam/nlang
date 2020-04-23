#pragma once

#include <common/heap/heap.hpp>

#include <utils/traits.hpp>

#include <type_traits>

namespace nlang {

class Environment {
public:
    enum class HeapType {
        Dynamic,
        Static,
    };

    template<HeapType heap_type = HeapType::Dynamic>
    Heap* GetHeap() {
        if constexpr (heap_type == HeapType::Dynamic) {
            return &dynamic_heap;
        } else if constexpr (heap_type == HeapType::Static) {
            return &static_heap;
        }
        return nullptr;
    }

private:
    Heap dynamic_heap;
    Heap static_heap;
};

}