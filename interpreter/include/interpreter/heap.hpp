#pragma once

#include "value.hpp"
#include "handle.hpp"

#include <utils/slot_storage.hpp>
#include <utils/macro.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>


namespace nlang {

class Heap {
public:
    Handle<HeapValue> Store(HeapValue* value) {
        auto slot = storage.Store(value);
        return Handle<HeapValue>(Handle<HeapValue>::BackingPrimitive(static_cast<void*>(slot)));
    }

public:
    SlotStorage<HeapValue> storage;
};

}
