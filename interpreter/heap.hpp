//
// Created by selya on 12.01.2020.
//

#ifndef NLANG_HEAP_HPP
#define NLANG_HEAP_HPP

#include "object.hpp"

#include <utils/defs.hpp>

#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <cstdlib>

#ifdef NLANG_PLATFORM_LINUX
#include <unistd.h>
#include <sys/mman.h>
#endif


namespace nlang {

using Address = uintptr_t;

class ObjectSlot {
    friend class Heap;

public:
    NLANG_FORCE_INLINE ObjectSlot()
        : location(Address(nullptr))
    {

    }

    [[nodiscard]] NLANG_FORCE_INLINE bool IsEmpty() const {
        return location == Address(nullptr);
    }

    NLANG_FORCE_INLINE Object& operator->() {
        return *reinterpret_cast<Object*>(location);
    }

    NLANG_FORCE_INLINE const Object& operator->() const {
        return *reinterpret_cast<Object*>(location);
    }

    NLANG_FORCE_INLINE Object& operator*() {
        return *reinterpret_cast<Object*>(location);
    }

    NLANG_FORCE_INLINE const Object& operator*() const {
        return *reinterpret_cast<Object*>(location);
    }

private:
    NLANG_FORCE_INLINE void Reset(Address location = Address(nullptr)) {
        this->location = location;
    }

    [[nodiscard]] NLANG_FORCE_INLINE Address Get() const {
        return location;
    }

    Address location;
};

class Heap {
private:
    class Page {
        friend class Heap;

    public:
        Page(const Page&) = delete;
        Page& operator=(const Page&) = delete;

        Page(Page&& page) {
            *this = std::move(page);
        }

        Page& operator=(Page&& page) noexcept {
            mem = page.mem;
            mem_size = page.mem_size;
            slots = page.slots;
            slots_count = page.slots_count;
            slots_count_free = page.slots_count_free;
            marks = std::move(page.marks);
            will_be_collected = page.will_be_collected;

            page.mem = nullptr;
            page.mem_size = 0;
            page.slots = nullptr;
            page.slots_count = 0;
            page.slots_count_free = 0;
            page.will_be_collected = false;

            return *this;
        }

        explicit Page(size_t page_size)
            : mem(nullptr)
            , mem_size(page_size)
            , slots(nullptr)
            , slots_count(0)
            , slots_count_free(slots_count)
            , will_be_collected(false)
        {
#ifdef NLANG_PLATFORM_LINUX
            mem = mmap(nullptr, mem_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (mem == MAP_FAILED) {
                // TODO error
                mem = nullptr;
                return;
            }
#else
            mem = std::aligned_alloc(mem_size, mem_size);
#endif
            void* aligned_mem = mem;
            size_t aligned_size = mem_size;
            if (!std::align(alignof(ObjectSlot), sizeof(ObjectSlot), aligned_mem, aligned_size)) {
                // TODO error
                return;
            }
            slots = (ObjectSlot*)aligned_mem;
            slots_count = aligned_size / sizeof(ObjectSlot);
            slots_count_free = slots_count;

            for (size_t i = 0; i < slots_count; ++i) {
                new (slots + i) ObjectSlot;
            }

            marks.resize(slots_count, false);
        }

        ~Page() {
            if (slots) {
                for (size_t i = 0; i < slots_count; ++i) {
                    (slots + i)->~ObjectSlot();
                }
            }
            if (mem) {
#ifdef NLANG_PLATFORM_LINUX
                munmap(mem, mem_size);
#else
                std::free(mem);
#endif
            }
        }

        NLANG_FORCE_INLINE ObjectSlot* Store(Address location) {
            if (!slots_count_free) {
                return nullptr;
            }
            for (size_t i = 0; i < slots_count; ++i) {
                if (slots[i].IsEmpty()) {
                    slots[i].Reset(location);
                    --slots_count_free;
                    return slots + i;
                }
            }
            return nullptr;
        }

        NLANG_FORCE_INLINE void FreeMarks() {
            for (auto&& mark : marks) {
                mark = false;
            }
        }

        NLANG_FORCE_INLINE bool TryMarkAchievableSlot(ObjectSlot*& slot) {
            if (will_be_collected) {
                // Slot was moved
                slot = reinterpret_cast<ObjectSlot*>(slot->Get());
                return false;
            }
            marks[slot - slots] = true;
            return true;
        }

        NLANG_FORCE_INLINE void ResetUnmarkedSlots() {
            for (size_t i = 0; i < slots_count; ++i) {
                if (marks[i]) {
                    slots[i].Reset();
                    ++slots_count_free;
                }
            }
        }

        [[nodiscard]] NLANG_FORCE_INLINE bool IsEmpty() const {
            return slots_count == slots_count_free;
        }

        [[nodiscard]] NLANG_FORCE_INLINE bool IsFull() const {
            return !slots_count_free;
        }

    private:
        void* mem;
        size_t mem_size;

        ObjectSlot* slots;
        size_t slots_count;
        size_t slots_count_free;

        std::vector<bool> marks;

        bool will_be_collected;
    };

public:
    Heap()
        : page_size(
#ifdef NLANG_PLATFORM_LINUX
            getpagesize()
#else
            4096
#endif
        )
    {

    }

    ObjectSlot* Store(Address location) {
        for (auto& [ptr, page] : storage) {
            auto slot = page.Store(location);
            if (slot) {
                return slot;
            }
        }
        Page page(page_size);
        auto result = storage.emplace(page.mem, std::move(page));
        if (result.second) {
            return result.first->second.Store(location);
        }
        return nullptr;
    }

// ================= GC part ===================
    void FreeMarks() {
        for (auto& [ptr, page] : storage) {
            page.FreeMarks();
        }
    }

    void PreparePagesCollect() {
        size_t free_slots_total = 0;
        for (auto& [ptr, page] : storage) {
            free_slots_total += page.slots_count_free;
        }
        bool ok = true;
        while (ok) {
            for (auto& [ptr, page] : storage) {
                size_t used_slots = page.slots_count - page.slots_count_free;
                if (used_slots <= free_slots_total - used_slots) {
                    free_slots_total -= used_slots;
                    ok = FreePage(page);
                    break;
                }
            }
        }
    }

    void MarkAchievableSlot(ObjectSlot*& slot) {
        for (size_t i = 0; i < 2; ++i) {
            if (storage[(void*) (size_t(slot) / page_size * page_size)].TryMarkAchievableSlot(slot)) {
                break;
            }
        }
    }

    void FinishPagesCollect() {
        std::unordered_map<void*, Page> new_storage;
        for (auto& [ptr, page] : storage) {
            if (!page.will_be_collected) {
                new_storage.emplace(ptr, std::move(page));
            }
        }
        std::swap(storage, new_storage);
    }

    void ResetUnmarkedSlots() {
        for (auto& [ptr, page] : storage) {
            page.ResetUnmarkedSlots();
        }
    }
// =============== GC part end ================

private:
    bool FreePage(Page& page) {
        page.will_be_collected = true;
        auto it = storage.begin();
        for (size_t i = 0; i < page.slots_count && !page.IsEmpty(); ++i) {
            if (it == storage.end()) {
                return false;
            }
            if (it->second.will_be_collected || it->second.IsFull()) {
                ++it;
                continue;
            }
            auto slot_to_move = page.slots + i;
            slot_to_move->Reset(Address(it->second.Store(slot_to_move->Get())));
        }
        return page.IsEmpty();
    }

    size_t page_size;
    std::unordered_map<void*, Page> storage;
};

}

#endif //NLANG_HEAP_HPP
