#pragma once

#include <interpreter/value.hpp>
#include <interpreter/handle.hpp>

#include <utils/containers/slot_storage.hpp>
#include <utils/macro.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>


namespace nlang {
/**
 * Functor for processing all values in heap
 */
class HeapValueHandler {
public:
    virtual void operator()(SlotPage<HeapValue>* page, SlotPage<HeapValue>::Slot* slot) = 0;
};
/**
 * Heap
 * Used for storing all runtime objects
 */
class Heap {
public:
    /**
     * Store the object in heap
     * @param value the object
     * @return Handle to stored object
     */
    Handle<HeapValue> Store(HeapValue* value) {
        auto* slot = storage.Store(value);
        return Handle<HeapValue>(Handle<HeapValue>::BackingPrimitive(static_cast<void*>(slot)));
    }

    /**
     * Execute function on each value in heap
     * @param function The function
     */
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
