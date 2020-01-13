//
// Created by selya on 12.01.2020.
//

#ifndef NLANG_HEAP_HPP
#define NLANG_HEAP_HPP

#include "object.hpp"

#include <vector>
#include <cstdint>
#include <unistd.h>
#include <sys/mman.h>
#include <memory>
#include <unordered_map>


namespace nlang {

class ObjectSlot {
    friend class Heap;

public:
    ObjectSlot()
        : object(nullptr)
    {

    }

    [[nodiscard]] bool IsEmpty() const {
        return !object;
    }

    Object& operator->() {
        return *object;
    }

    const Object& operator->() const {
        return *object;
    }

    Object& operator*() {
        return *object;
    }

    const Object& operator*() const {
        return *object;
    }

private:
    void Reset(Object* object = nullptr) {
        this->object = object;
    }

    Object* object;
};

class Heap {
private:
    class Page {
        friend class Heap;

    public:
        explicit Page(size_t page_size)
            : mem(nullptr)
            , mem_size(page_size)
            , slots(nullptr)
            , size(0)
            , free_slots(size)
        {
            mem = mmap(nullptr, mem_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (mem == MAP_FAILED) {
                // TODO error
                mem = nullptr;
                return;
            }
            void* aligned_mem = mem;
            size_t aligned_size = mem_size;
            if (!std::align(alignof(ObjectSlot), sizeof(ObjectSlot), aligned_mem, aligned_size)) {
                // TODO error
                return;
            }
            slots = (ObjectSlot*)aligned_mem;
            size = aligned_size / sizeof(ObjectSlot);
            free_slots = size;

            for (size_t i = 0; i < size; ++i) {
                new (slots + i) ObjectSlot;
            }

            marks.resize(size, false);
        }

        ~Page() {
            if (slots) {
                for (size_t i = 0; i < size; ++i) {
                    (slots + i)->~ObjectSlot();
                }
            }
            if (mem) {
                munmap(mem, mem_size);
            }
        }

        ObjectSlot* Store(Object* object) {
            if (!free_slots) {
                return nullptr;
            }
            for (size_t i = 0; i < size; ++i) {
                if (slots[i].IsEmpty()) {
                    slots[i].Reset(object);
                    --free_slots;
                    return slots + i;
                }
            }
            return nullptr;
        }

        void FreeMarks() {
            for (auto&& mark : marks) {
                mark = false;
            }
        }

        ObjectSlot* MarkSlot(ObjectSlot* slot) {
            marks[slot - slots] = true;
            return slot;// TODO new location
        }

        void ResetUnmarkedSlots() {
            for (size_t i = 0; i < size; ++i) {
                if (marks[i]) {
                    slots[i].Reset();
                    ++free_slots;
                }
            }
        }

        [[nodiscard]] bool IsFree() const {
            return size == free_slots;
        }

    private:
        void* mem;
        size_t mem_size;

        ObjectSlot* slots;
        size_t size;
        size_t free_slots;

        std::vector<bool> marks;
    };

public:
    Heap()
        : page_size(getpagesize())
    {

    }

    ObjectSlot* Store(Object* object) {
        for (auto& [ptr, page] : storage) {
            auto slot = page.Store(object);
            if (slot) {
                return slot;
            }
        }
        Page page(page_size);
        storage.emplace(page.mem, std::move(page));
    }

    void FreeMarks() {
        for (auto& [ptr, page] : storage) {
            page.FreeMarks();
        }
    }

    void MarkSlot(ObjectSlot* slot) {
        storage[(void*)(size_t(slot) / page_size * page_size)].MarkSlot(slot);
    }

    void ResetUnmarkedSlots() {
        for (auto& [ptr, page] : storage) {
            page.ResetUnmarkedSlots();
        }
    }

private:
    size_t page_size;
    std::unordered_map<void*, Page> storage;
};

}

#endif //NLANG_HEAP_HPP
