#pragma once

#include <interpreter/stack_frame.hpp>

#include <interpreter/heap.hpp>

#include <cstddef>
#include <functional>

namespace nlang {

/**
 * Garbage Collector interface
 */
class IGC {
public:
    /**
     * Called to possibly trigger the garbage collection
     */
    virtual void Collect() = 0;
};

/**
 * Basic single-pass mark-sweep-compact garbage collector
 */
class BasicGC : public IGC {
public:
    using SlotMark = SlotPage<HeapValue>::Slot::Mark;
    BasicGC() = delete;

    BasicGC(StackFrame* the_stack, Heap* the_heap) : vm_stack(the_stack), vm_heap(the_heap) {}

    virtual void Collect() override {
        Mark();
        Sweep();
        Compact();
    }

protected:
    /**
     * Recursively marks all reachable objects
     */
    virtual void Mark() {
        if (!vm_stack)
            return;

        std::function<void(Handle<Value>)> node_marker;

        node_marker = [&node_marker](Handle<Value> value) {
            if (!value.Is<HeapValue>())
                return;

            Handle<HeapValue> heap_value = value.As<HeapValue>();
            auto* value_slot = heap_value.GetSlot();
            if (value_slot->GetMark() == SlotMark::WHITE) {
                value_slot->SetMark(SlotMark::GREY);
                value_slot->Get()->ForEachReference(node_marker);
                value_slot->SetMark(SlotMark::BLACK);
            }
        };

        auto* current_sf = vm_stack;
        while (current_sf) {
            node_marker(current_sf->context);
            node_marker(current_sf->function);
            for (Handle<Value>* current = current_sf->arguments; current < static_cast<void*>(current_sf->next); ++current) {
                node_marker(current->As<HeapValue>());
            }

            current_sf = current_sf->prev;
        }
    }

    /**
     * Deletes the unreachable (= unused) objects
     */
    virtual void Sweep() {
        vm_heap->ForEachValue([](SlotPage<HeapValue>* page, SlotPage<HeapValue>::Slot* slot) {
            if (slot->GetMark() != SlotMark::BLACK) {
                slot->SetMark(SlotMark::WHITE);
                delete slot->Get();
                page->Release(slot);
            }
            else {
                slot->SetMark(SlotMark::WHITE);
            }
        });
    }

    /**
     * Defragments the heap to bring occupied slots closed to each other
     */
    virtual void Compact() {
        vm_heap->storage.Defragment();
    }

    StackFrame* vm_stack;
    Heap* vm_heap;
};

/**
 * Two-pass mark-sweep-compact GC
 */
class TwoPassGC : public BasicGC {
    enum class PassType : uint8_t {
        MARK = 0,
        SWEEP_COMPACT
    };

public:
    virtual void Collect() override {
        if (next_pass == PassType::MARK) {
            Mark();
            next_pass = PassType::SWEEP_COMPACT;
        }
        else {
            Sweep();
            Compact();
            next_pass = PassType::SWEEP_COMPACT;
        }
    }


private:
    PassType next_pass = PassType::MARK;
};

}