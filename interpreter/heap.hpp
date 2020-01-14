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

#if defined(NLANG_PLATFORM_LINUX)
#include <unistd.h>
#include <sys/mman.h>
#elif defined(NLANG_PLATFORM_WINDOWS)
// Sets minimal API level requirement to Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <Windows.h>
#endif


namespace nlang {

using Address = uintptr_t;

class ObjectSlot {
    friend class Heap;

public:
    NLANG_FORCE_INLINE ObjectSlot()
        : location((Address) nullptr)
    {

    }

    [[nodiscard]] NLANG_FORCE_INLINE bool IsEmpty() const {
        return location == (Address) nullptr;
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
    NLANG_FORCE_INLINE void Reset(Address location = (Address) nullptr) {
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
            raw_data_ = other.raw_data_;
            raw_size_ = other.raw_size_;
            other.raw_data_ = nullptr;
            other.raw_size_ = 0;
        }

        NLANG_FORCE_INLINE Page& operator=(Page&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            raw_data_ = other.raw_data_;
            raw_size_ = other.raw_size_;
            other.raw_data_ = nullptr;
            other.raw_size_ = 0;
            return *this;
        }

        NLANG_FORCE_INLINE Page()
            : raw_data_(nullptr)
            , raw_size_(GetPageSize())
        {
#if defined(NLANG_PLATFORM_LINUX)
            raw_data_ = mmap(nullptr, raw_size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (raw_data_ == MAP_FAILED) {
                throw std::bad_alloc();
            }
#elif defined(NLANG_PLATFORM_WINDOWS)
            raw_data_= VirtualAlloc(NULL, raw_size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!raw_data_) {
                throw std::bad_alloc();
            }
#else
            raw_data_ = aligned_alloc(raw_size_, raw_size_);
            if (!raw_data_) {
                throw std::bad_alloc();
            }
#endif
        }

        NLANG_FORCE_INLINE ~Page() {
            if (raw_data_) {
#if defined(NLANG_PLATFORM_LINUX)
                munmap(raw_data_, raw_size_);
#elif defined(NLANG_PLATFORM_WINDOWS)
                VirtualFree(raw_data_, 0, MEM_RELEASE);
#else
                free(raw_data_);
#endif
            }
        }

        [[nodiscard]] NLANG_FORCE_INLINE void* raw_data() const {
            return raw_data_;
        }

        [[nodiscard]] NLANG_FORCE_INLINE size_t raw_size() const {
            return raw_size_;
        }

        NLANG_FORCE_INLINE static size_t GetPageSize() {
            static size_t page_size;
#if defined(NLANG_PLATFORM_LINUX)
            page_size = getpagesize();
#elif defined(NLANG_PLATFORM_WINDOWS)
            SYSTEM_INFO info;
            GetNativeSystemInfo(&info);
            page_size = info.dwPageSize;
#else
            page_size = 4096;
#endif
            return page_size;
        }

    protected:
        void* raw_data_;
        size_t raw_size_;
    };

    template<typename T, bool default_construct_destruct = true>
    class TypedPage : public Page {
    public:
        TypedPage(const TypedPage&) = delete;
        TypedPage& operator=(const TypedPage&) = delete;

        NLANG_FORCE_INLINE TypedPage(TypedPage&& other) noexcept : Page(std::move(other)) {
            typed_data = other.typed_data;
            typed_size = other.typed_size;
            other.typed_data = nullptr;
            other.typed_size = 0;
        }

        NLANG_FORCE_INLINE TypedPage& operator=(TypedPage&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            typed_data = other.typed_data;
            typed_size = other.typed_size;
            other.typed_data = nullptr;
            other.typed_size = 0;
            Page::operator=(std::move(other));
            return *this;
        }

        NLANG_FORCE_INLINE TypedPage() : Page() {
            void* aligned_data = raw_data_;
            size_t aligned_size = raw_size_;
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

        [[nodiscard]] NLANG_FORCE_INLINE T* data() const {
            return typed_data;
        }

        [[nodiscard]] NLANG_FORCE_INLINE size_t size() const {
            return typed_size;
        }

        NLANG_FORCE_INLINE T& operator[](size_t index) {
            return typed_data[index];
        }

        NLANG_FORCE_INLINE const T& operator[](size_t index) const {
            return typed_data[index];
        }

        NLANG_FORCE_INLINE T* begin() {
            return typed_data;
        }

        [[nodiscard]] NLANG_FORCE_INLINE T* const begin() const {
            return typed_data;
        }

        NLANG_FORCE_INLINE T* end() {
            return typed_data + typed_size;
        }

        [[nodiscard]] NLANG_FORCE_INLINE T* const end() const {
            return typed_data + typed_size;
        }

    protected:
        T* typed_data;
        size_t typed_size;
    };

    class PageOfSlots : public TypedPage<ObjectSlot> {
        friend class Heap;

    public:
        PageOfSlots(const PageOfSlots&) = delete;
        PageOfSlots& operator=(const PageOfSlots&) = delete;

        PageOfSlots(PageOfSlots&& other) noexcept : TypedPage<ObjectSlot>(std::move(other)) {
            free_ranges = std::move(other.free_ranges);
            marks = std::move(other.marks);
            will_be_collected = other.will_be_collected;
            other.will_be_collected = false;
        }

        PageOfSlots& operator=(PageOfSlots&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            free_ranges = std::move(other.free_ranges);
            marks = std::move(other.marks);
            will_be_collected = other.will_be_collected;
            other.will_be_collected = false;
            TypedPage<ObjectSlot>::operator=(std::move(other));
            return *this;
        }

        explicit PageOfSlots()
            : free_ranges({ {data(), size() } })
            , marks(size(), false)
            , will_be_collected(false)
        {

        }

        NLANG_FORCE_INLINE ObjectSlot* Store(Address location) {
            if (location == (Address) nullptr) {
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
            if (slot < data() || slot >= data() + size()) {
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
            if (slot < data() || slot >= data() + size()) {
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
            marks[slot - data()] = true;
            return true;
        }

        NLANG_FORCE_INLINE void ResetUnmarkedSlots() {
            for (size_t i = 0; i < size(); ++i) {
                if (!marks[i] && !data()[i].IsEmpty()) {
                    Reset(data() + i);
                }
            }
        }

        [[nodiscard]] NLANG_FORCE_INLINE bool IsEmpty() const {
            return free_ranges.size() == 1 && free_ranges.begin()->second == size();
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
            return size() - GetFreeSlotsCount();
        }

    private:
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
        auto result = storage.emplace(page.raw_data(), std::move(page));
        if (result.second) {
            return result.first->second.Store(location);
        }
        return nullptr;
    }

    [[nodiscard]] size_t GetPagesCount() const {
        return storage.size();
    }

    [[nodiscard]] size_t GetOccupiedSlotsCount() const {
        size_t occupied_slots = 0;
        for (auto& [ptr, page] : storage) {
            occupied_slots += page.GetOccupiedSlotsCount();
        }
        return occupied_slots;
    }

    [[nodiscard]] size_t GetFreeSlotsCount() const {
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
            if (free_slots >= page_from.size()) {
                free_slots -= page_from.GetFreeSlotsCount();
                page_from.will_be_collected = true;
                for (auto& slot : page_from) {
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
