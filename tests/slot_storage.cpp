#include <catch2/catch.hpp>

#include <utils/slot_storage.hpp>

#include <vector>

TEST_CASE("slot storage tests") {
    using namespace nlang::utils;

    SlotStorage<int> storage;

    std::vector<int> ints;
    std::vector<SlotStorage<int>::Slot*> slots;

    constexpr int count = 100000;

    ints.reserve(count);
    slots.reserve(count);

    for (int i = 0; i < count; ++i) {
        ints.push_back(i);
        slots.push_back(storage.Store(&ints.back()));
    }

    REQUIRE(storage.size() == count);
    REQUIRE(storage.capacity() >= storage.size());
    REQUIRE(storage.capacity() <= storage.size() + Page::size());

    for (int i = 0; i < count; i += 2) {
        slots[i]->SetMark(SlotStorage<int>::Slot::Mark::BLACK);
    }

    size_t capacity = storage.capacity();
    size_t size = storage.capacity();

    storage.ForEachSlot([](SlotPage<int>* page, SlotStorage<int>::Slot* slot) {
        if (slot->GetMark() != SlotStorage<int>::Slot::Mark::BLACK) {
            page->Release(slot);
        }
    });

    REQUIRE(storage.capacity() == capacity);
    REQUIRE(storage.size() < size);
    capacity = storage.capacity();
    size = storage.size();

    storage.Defragment();

    REQUIRE(storage.capacity() == capacity);
    REQUIRE(storage.size() > size);
    capacity = storage.capacity();
    size = storage.size();

    size_t moved_count = 0;
    for (auto& slot : slots) {
        if (slot->IsMoved()) {
            slot = slot->GetNewLocation();
            ++moved_count;
        }
    }
    REQUIRE(moved_count > 0);

    storage.ForEachSlot([](SlotPage<int>* page, SlotStorage<int>::Slot* slot) {
        if (slot->GetMark() == SlotStorage<int>::Slot::Mark::MOVED) {
            page->Release(slot);
        }
    });

    REQUIRE(storage.capacity() == capacity);
    REQUIRE(storage.size() < size);
    capacity = storage.capacity();
    size = storage.size();

    storage.FreeEmptyPages();

    REQUIRE(storage.capacity() < capacity);
    REQUIRE(storage.size() == size);
    REQUIRE(storage.size() == count / 2);

    storage.ForEachSlot([](SlotPage<int>* page, SlotStorage<int>::Slot* slot) {
        REQUIRE(slot->GetMark() != SlotStorage<int>::Slot::Mark::MOVED);
        REQUIRE(slot->GetMark() == SlotStorage<int>::Slot::Mark::BLACK);
        REQUIRE(*slot->Get() % 2 == 0);
    });
}