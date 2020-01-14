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
#include <new>

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

    [[nodiscard]] NLANG_FORCE_INLINE Address Get() const {
        return location;
    }

private:
    NLANG_FORCE_INLINE void Reset(Address location = Address(nullptr)) {
        this->location = location;
    }

    Address location;
};

class Heap {
private:
    class Page {
    public:
        Page(const Page&) = delete;
        Page& operator=(const Page&) = delete;

        NLANG_FORCE_INLINE Page(Page&& other) noexcept {
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }

        NLANG_FORCE_INLINE Page& operator=(Page&& other) noexcept {
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
            return *this;
        }

        NLANG_FORCE_INLINE Page()
            : data(nullptr)
            , size(GetPageSize())
        {
#ifdef NLANG_PLATFORM_LINUX
            data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (data == MAP_FAILED) {
                throw std::bad_alloc();
            }
#else
            data = std::aligned_alloc(size, size);
            if (!data) {
                throw std::bad_alloc();
            }
#endif
        }

        NLANG_FORCE_INLINE ~Page() {
            if (data) {
#ifdef NLANG_PLATFORM_LINUX
                munmap(data, size);
#else
                std::free(data);
#endif
            }
        }

        [[nodiscard]] NLANG_FORCE_INLINE void* GetData() const {
            return data;
        }

        [[nodiscard]] NLANG_FORCE_INLINE size_t GetSize() const {
            return size;
        }

        NLANG_FORCE_INLINE static size_t GetPageSize() {
#ifdef NLANG_PLATFORM_LINUX
            return getpagesize();
#else
            return 4096;
#endif
        }

    protected:
        void* data;
        size_t size;
    };

    template<typename T, bool default_construct_destruct = true>
    class TypedPage : public Page {
    public:
        TypedPage(const TypedPage&) = delete;
        TypedPage& operator=(const TypedPage&) = delete;

        NLANG_FORCE_INLINE TypedPage(TypedPage&& other) noexcept {
            data = other.data;
            size = other.size;
            typed_data = other.typed_data;
            typed_size = other.typed_size;
            other.data = nullptr;
            other.size = 0;
            other.typed_data = nullptr;
            other.typed_size = 0;
        }

        NLANG_FORCE_INLINE TypedPage& operator=(TypedPage&& other) noexcept {
            data = other.data;
            size = other.size;
            typed_data = other.typed_data;
            typed_size = other.typed_size;
            other.data = nullptr;
            other.size = 0;
            other.typed_data = nullptr;
            other.typed_size = 0;
            return *this;
        }

        NLANG_FORCE_INLINE TypedPage() : Page() {
            void* aligned_data = data;
            size_t aligned_size = size;
            if (!std::align(alignof(T), sizeof(T), aligned_data, aligned_size)) {
                throw std::bad_alloc();
            }
            typed_data = reinterpret_cast<T*>(aligned_data);
            typed_size = aligned_size / sizeof(T);

            if constexpr (default_construct_destruct) {
                for (size_t i = 0; i < typed_size; ++i) {
                    new (typed_data + i) T;
                }
            }
        }

        NLANG_FORCE_INLINE ~TypedPage() {
            if constexpr (default_construct_destruct) {
                if (typed_data) {
                    for (size_t i = 0; i < typed_size; ++i) {
                        (typed_data + i)->~T();
                    }
                }
            }
        }

        [[nodiscard]] NLANG_FORCE_INLINE T* GetTypedData() const {
            return typed_data;
        }

        [[nodiscard]] NLANG_FORCE_INLINE size_t GetTypedSize() const {
            return typed_size;
        }

    protected:
        T* typed_data;
        size_t typed_size;
    };

    class PageOfSlots {
        friend class Heap;

    public:
        PageOfSlots(const PageOfSlots&) = delete;
        PageOfSlots& operator=(const PageOfSlots&) = delete;

        PageOfSlots(PageOfSlots&& other) noexcept {
            *this = std::move(other);
        }

        PageOfSlots& operator=(PageOfSlots&& other) noexcept {
            page = std::move(other.page);
            free_ranges = std::move(other.free_ranges);
            marks = std::move(other.marks);
            will_be_collected = other.will_be_collected;
            other.will_be_collected = false;
            return *this;
        }

        explicit PageOfSlots()
            : free_ranges({ { page.GetTypedData(), page.GetTypedSize() } })
            , marks(page.GetTypedSize(), false)
            , will_be_collected(false)
        {

        }

        NLANG_FORCE_INLINE ObjectSlot* Store(Address location) {
            if (location == Address(nullptr)) {
                throw std::runtime_error("storing empty address has no sense");
            }
            if (IsFull()) {
                return nullptr;
            }
            auto it = free_ranges.begin();
            auto& [slot, count] = *it;
            ObjectSlot* slot_to_return = slot + count - 1;
            slot_to_return->Reset(location);
            --count;
            if (!count) {
                free_ranges.erase(it);
            }
            return slot_to_return;
        }

        NLANG_FORCE_INLINE void Reset(ObjectSlot* slot) {
            if (slot < page.GetTypedData() || slot >= page.GetTypedData() + page.GetTypedSize()) {
                throw std::out_of_range("slot not from this page");
            }
            if (slot->IsEmpty()) {
                throw std::runtime_error("can't reset empty slot");
            }
            if (free_ranges.empty()) {
                free_ranges.emplace(slot, 1);
                return;
            }
            auto greater = free_ranges.upper_bound(slot);
            std::optional<decltype(free_ranges)::iterator> less;
            ObjectSlot* start = slot;
            size_t size = 1;
            if (greater != free_ranges.end()) {
                if (greater != free_ranges.begin()) {
                    less = std::prev(greater);
                }
                if (greater->first == slot + 1) {
                    size += greater->second;
                    free_ranges.erase(greater);
                }
            } else {
                less = std::prev(greater);
            }
            if (less) {
                if ((*less)->first + (*less)->second == slot) {
                    start = (*less)->first;
                    size += (*less)->second;
                    free_ranges.erase(*less);
                }
            }
            free_ranges.emplace(start, size);
            slot->Reset();
        }

        NLANG_FORCE_INLINE void FreeMarks() {
            for (auto&& mark : marks) {
                mark = false;
            }
        }

        NLANG_FORCE_INLINE bool TryMarkOrUpdateAchievableSlot(ObjectSlot*& slot) {
            if (slot < page.GetTypedData() || slot >= page.GetTypedData() + page.GetTypedSize()) {
                throw std::out_of_range("slot not from this page");
            }
            if (slot->IsEmpty()) {
                throw std::runtime_error("can't mark empty slot");
            }
            if (will_be_collected) {
                // Slot was moved
                slot = reinterpret_cast<ObjectSlot*>(slot->Get());
                return false;
            }
            marks[slot - page.GetTypedData()] = true;
            return true;
        }

        NLANG_FORCE_INLINE void ResetUnmarkedSlots() {
            for (size_t i = 0; i < page.GetTypedSize(); ++i) {
                if (!marks[i] && !page.GetTypedData()[i].IsEmpty()) {
                    Reset(page.GetTypedData() + i);
                }
            }
        }

        [[nodiscard]] NLANG_FORCE_INLINE bool IsEmpty() const {
            return free_ranges.size() == 1 && free_ranges.begin()->second == page.GetTypedSize();
        }

        [[nodiscard]] NLANG_FORCE_INLINE bool IsFull() const {
            return free_ranges.empty();
        }

        [[nodiscard]] NLANG_FORCE_INLINE size_t GetFreeSlotsCount() const {
            size_t free_count = 0;
            for (auto& [slot, count] : free_ranges) {
                free_count += count;
            }
            return free_count;
        }

        [[nodiscard]] NLANG_FORCE_INLINE size_t GetOccupiedSlotsCount() const {
            return page.GetTypedSize() - GetFreeSlotsCount();
        }

    private:
        TypedPage<ObjectSlot> page;
        std::map<ObjectSlot*, size_t> free_ranges;

        std::vector<bool> marks;

        bool will_be_collected;
    };

public:
    Heap() = default;
    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;

    Heap(Heap&& other) noexcept {
        storage = std::move(other.storage);
    }

    Heap& operator=(Heap&& other) noexcept {
        storage = std::move(other.storage);
        return *this;
    }

    ObjectSlot* Store(Address location) {
        for (auto& [ptr, page] : storage) {
            auto slot = page.Store(location);
            if (slot) {
                return slot;
            }
        }
        PageOfSlots page;
        auto result = storage.emplace(page.page.GetData(), std::move(page));
        if (result.second) {
            return result.first->second.Store(location);
        }
        return nullptr;
    }

    size_t GetPagesCount() const {
        return storage.size();
    }

    size_t GetOccupiedSlotsCount() const {
        size_t occupied_slots = 0;
        for (auto& [ptr, page] : storage) {
            occupied_slots += page.GetOccupiedSlotsCount();
        }
        return occupied_slots;
    }

    size_t GetFreeSlotsCount() const {
        size_t free_slots = 0;
        for (auto& [ptr, page] : storage) {
            free_slots += page.GetFreeSlotsCount();
        }
        return free_slots;
    }

    // ================= GC part ===================
private:
    void FreeMarks() {
        for (auto& [ptr, page] : storage) {
            page.FreeMarks();
        }
    }

    void BeginPagesCollect() {
        {
            // collect all empty pages
            std::unordered_map<void*, PageOfSlots> new_storage;
            for (auto& [ptr, page] : storage) {
                if (!page.IsEmpty()) {
                    new_storage.emplace(ptr, std::move(page));
                }
            }
            std::swap(storage, new_storage);
        }

        size_t free_slots = GetFreeSlotsCount();

        auto copy_to_it = storage.begin();

        for (auto copy_from_it = storage.begin(); copy_from_it != storage.end(); ++copy_from_it) {
            if (copy_from_it == copy_to_it || copy_from_it->second.IsFull()) {
                continue;
            }
            PageOfSlots& page_from = copy_from_it->second;
            if (free_slots >= page_from.page.GetTypedSize()) {
                free_slots -= page_from.GetFreeSlotsCount();
                page_from.will_be_collected = true;
                for (size_t i = 0; i < page_from.page.GetTypedSize(); ++i) {
                    ObjectSlot& slot = page_from.page.GetTypedData()[i];
                    if (slot.IsEmpty()) {
                        continue;
                    }
                    while (true) {
                        if (copy_to_it == storage.end()) {
                            throw std::runtime_error("unexpected end of free space for slots move");
                        }
                        if (copy_to_it->second.will_be_collected || copy_to_it->second.IsFull()) {
                            ++copy_to_it;
                        } else {
                            break;
                        }
                    }
                    slot.Reset(Address(copy_to_it->second.Store(slot.Get())));
                    --free_slots;
                }
            }
        }
    }

public:
    void PreGC() {
        FreeMarks();
        BeginPagesCollect();
    }

    void MarkOrUpdateAchievableSlot(ObjectSlot*& slot) {
        for (size_t i = 0; i < 2; ++i) {
            if (storage.at(
                (void*) (size_t(slot) / Page::GetPageSize() * Page::GetPageSize())).TryMarkOrUpdateAchievableSlot(slot)) {
                break;
            }
        }
    }

    void PostGC() {
        EndPagesCollect();
        ResetUnmarkedSlots();
    }

private:
    void EndPagesCollect() {
        std::unordered_map<void*, PageOfSlots> new_storage;
        for (auto& [ptr, page] : storage) {
            if (!page.will_be_collected) {
                new_storage.emplace(ptr, std::move(page));
            }
        }
        std::swap(storage, new_storage);
    }

    void ResetUnmarkedSlots() {
        for (auto& [ptr, page] : storage) {
            if (!page.will_be_collected) {
                page.ResetUnmarkedSlots();
            }
        }
    }
// =============== GC part end ================

private:
    std::unordered_map<void*, PageOfSlots> storage;
};

}

#endif //NLANG_HEAP_HPP
