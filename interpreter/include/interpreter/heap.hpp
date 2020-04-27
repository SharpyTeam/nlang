#pragma once

#include <interpreter/value.hpp>
#include <interpreter/handle.hpp>

#include <utils/containers/slot_storage.hpp>
#include <utils/macro.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>


namespace nlang {

class HeapValueHandler {
public:
    virtual void operator()(SlotPage<HeapValue>* page, SlotPage<HeapValue>::Slot* slot) = 0;
};

class Heap {
public:
    Handle<HeapValue> Store(HeapValue* value) {
        auto* slot = storage.Store(value);
        return Handle<HeapValue>(Handle<HeapValue>::BackingPrimitive(static_cast<void*>(slot)));
    }

    void ForEachValue(std::function<void(SlotPage<HeapValue>*, SlotPage<HeapValue>::Slot*)> function) {
        storage.ForEachSlot(function);
    }

    virtual ~Heap() {
        ForEachValue([](SlotPage<HeapValue>* page, SlotPage<HeapValue>::Slot* slot) {
            delete slot->Get();
        });
    }

public:
    SlotStorage<HeapValue> storage;
};


}
