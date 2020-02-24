#pragma once

#include "forward_list_view.hpp"
#include "page.hpp"

#include <algorithm>

namespace nlang::utils {

template<typename T>
class SlotPage : public utils::ForwardListView<SlotPage<T>>::NodeHeader {
public:
    class FreeSlot : public utils::ForwardListView<FreeSlot>::NodeHeader {
    public:
        FreeSlot() {}
    };

    class Slot {
    public:
        enum class Mark : uintptr_t {
            WHITE = 0b00,
            GREY  = 0b01,
            BLACK = 0b10,
            MOVED = 0b11,
        };

        Slot(T* value) : data(value) {}

        T* Get() const {
            return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(data) & ~(uintptr_t)0b11u);
        }

        Mark GetMark() const {
            return static_cast<Mark>(reinterpret_cast<uintptr_t>(data) & (uintptr_t)0b11u);
        }

        void SetMark(Mark mark) {
            NLANG_ASSERT(mark != Mark::MOVED);
            SetMarkImpl(mark);
        }

        Slot* MoveTo(SlotPage<T>* page) {
            NLANG_ASSERT(!IsMoved());
            Slot* typed_slot = nullptr;
            if (page) {
                typed_slot = page->Store(Get());
                typed_slot->SetMark(GetMark());
            }
            data = typed_slot;
            SetMarkImpl(Mark::MOVED);
            return typed_slot;
        }

        bool IsMoved() const {
            return GetMark() == Mark::MOVED;
        }

        Slot* GetNewLocation() const {
            NLANG_ASSERT(GetMark() == Mark::MOVED);
            return reinterpret_cast<Slot*>(reinterpret_cast<uintptr_t>(data) & ~(uintptr_t)0b11u);
        }

    private:
        void SetMarkImpl(Mark mark) {
            data = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(data) & ~(uintptr_t)0b11u) | (uintptr_t)mark);
        }

        void* data;
    };

    static SlotPage* New() {
        utils::Page* page = utils::Page::Allocate();
        SlotPage* slot_page = new (page) SlotPage;
        return slot_page;
    }

    static void Release(SlotPage* slot_page) {
        utils::Page* page = static_cast<utils::Page*>(static_cast<void*>(slot_page));
        slot_page->~SlotPage();
        utils::Page::Free(page);
    }

    Slot* Store(T* value) {
        NLANG_ASSERT(!full());
        FreeSlot* slot = &free_slots.front();
        free_slots.pop_front();
        ++size_;
        return new (slot) Slot(value);
    }

    void Release(const Slot* slot) {
        NLANG_ASSERT(std::find_if(free_slots.begin(), free_slots.end(), [&](const FreeSlot& s) { return static_cast<const void*>(&s) == static_cast<const void*>(slot); }) == free_slots.end());
        NLANG_ASSERT(slot >= begin() && slot < end());
        free_slots.push_front(*new (const_cast<Slot*>(slot)) FreeSlot);
        --size_;
    }

    template<typename F>
    void ForEachSlot(F&& handler) {
        free_slots.sort([](const FreeSlot& a, const FreeSlot& b) { return &a < &b; });
        auto free_slot_it = free_slots.cbegin();
        for (auto slot = begin(); slot != end(); ++slot) {
            while (free_slot_it != free_slots.cend() && static_cast<const void*>(&*free_slot_it) < static_cast<const void*>(slot)) {
                ++free_slot_it;
            }
            if (free_slot_it != free_slots.cend() && static_cast<const void*>(&*free_slot_it) == static_cast<const void*>(slot)) {
                continue;
            }
            handler(this, slot);
        }
    }

    size_t capacity() const {
        return end() - begin();
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    bool full() const {
        return free_slots.empty();
    }

private:
    SlotPage() : size_(0) {
        for (auto& slot : *this) {
            free_slots.push_front(*new (&slot) FreeSlot);
        }
    }

    ~SlotPage() {}

    Slot* begin() const {
        return static_cast<Slot*>(static_cast<void*>(const_cast<SlotPage*>(this) + 1));
    }

    Slot* end() const {
        NLANG_ASSERT((utils::Page::size() - sizeof(SlotPage)) % sizeof(Slot) == 0);
        return begin() + (utils::Page::size() - sizeof(SlotPage)) / sizeof(Slot);
    }

private:
    utils::ForwardListView<FreeSlot> free_slots;
    size_t size_;
};


template<typename T>
class SlotStorage {
public:
    using Slot = typename SlotPage<T>::Slot;

    SlotStorage()
        : size_(0)
        , capacity_(0)
    {

    }

    ~SlotStorage() {
        FreePages(false);
    }

    Slot* Store(T* value) {
        if (pages.empty()) {
            pages.push_front(*SlotPage<T>::New());
            capacity_ += pages.front().capacity();
        }
        SlotPage<T>& page = pages.front();
        Slot* slot = page.Store(value);
        ++size_;
        if (page.full()) {
            pages.pop_front();
            full_pages.push_front(page);
        }
        return slot;
    }

    void Defragment() {
        // sort descending by size, optional
        pages.sort([](const SlotPage<T>& a, const SlotPage<T>& b) {
            return b.size() < a.size();
        });

        size_t slots_to = 0;
        size_t slots_from = size_;

        auto page_from_it = pages.begin();

        while (page_from_it != pages.end() && slots_to < slots_from) {
            slots_to += page_from_it->capacity() - page_from_it->size();
            slots_from -= page_from_it->size();
            ++page_from_it;
        }

        auto move_slot = [&](Slot* slot) -> Slot* {
            SlotPage<T>& page = pages.front();
            size_ -= page.size();
            Slot* new_slot = slot->MoveTo(&page);
            size_ += page.size();
            if (page.full()) {
                full_pages.splice_after(full_pages.cbefore_begin(), pages, pages.cbefore_begin(), std::next(pages.begin()));
            }
            return new_slot;
        };

        for ( ; page_from_it != pages.end(); ++page_from_it) {
            page_from_it->ForEachSlot([&](SlotPage<T>* page, Slot* slot) {
                move_slot(slot);
            });
        }
    }

    void FreeEmptyPages() {
        FreePages(true);
    }

    template<typename F>
    void ForEachSlot(F&& handler) {
        for (auto& page : pages) {
            size_ -= page.size();
            page.ForEachSlot(std::forward<F>(handler));
            size_ += page.size();
        }

        auto prev = full_pages.before_begin();
        auto current = full_pages.begin();
        while (current != full_pages.end()) {
            size_ -= current->size();
            current->ForEachSlot(std::forward<F>(handler));
            size_ += current->size();
            if (!current->full()) {
                ++current;
                pages.splice_after(pages.cbefore_begin(), full_pages, prev, current);
            } else {
                ++prev;
                ++current;
            }
        }
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return capacity_;
    }

private:
    void FreePages(bool empty_only) {
        SlotPage<T>* page = nullptr;
        pages.remove_if([&](const SlotPage<T>& p) {
            if (!empty_only || p.empty()) {
                if (page) {
                    capacity_ -= page->capacity();
                    SlotPage<T>::Release(page);
                }
                page = const_cast<SlotPage<T>*>(&p);
                return true;
            }
            return false;
        });
        if (page) {
            capacity_ -= page->capacity();
            SlotPage<T>::Release(page);
        }
    }

private:
    utils::ForwardListView<SlotPage<T>> pages;
    utils::ForwardListView<SlotPage<T>> full_pages;
    size_t size_;
    size_t capacity_;
};

}