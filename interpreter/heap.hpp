//
// Created by selya on 12.01.2020.
//

#ifndef NLANG_HEAP_HPP
#define NLANG_HEAP_HPP

#include "page.hpp"

#include "handle.hpp"
#include "value.hpp"

#include <utils/defs.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>


namespace nlang {

class Heap {
private:
    struct alignas(sizeof(void*)) HeapEntry {
        // Implementation-defined and potentially UB, but on MSVC, GCC and Clang type punning through union is allowed
        union {
            HeapValue* value;
            HeapEntry* next;
            uintptr_t a;
        };
    };

    struct alignas(sizeof(void*) * 4) PageHeader {
        PageHeader* prev;
        PageHeader* next;
        HeapEntry* entry_empty_first;
        size_t claimed_count;

        PageHeader()
            : prev(nullptr)
            , next(nullptr)
            , entry_empty_first(begin())
            , claimed_count(0)
        {
            for (HeapEntry& entry : *this) {
                entry.next = &entry + 1;
            }
            (end() - 1)->next = nullptr;
        }

        bool IsFull() const {
            return entry_empty_first == nullptr;
        }

        bool IsEmpty() const {
            return claimed_count == 0;
        }

        HeapEntry* ClaimEntry(HeapValue* value) {
            NLANG_ASSERT(!IsFull());

            auto entry = entry_empty_first;
            entry_empty_first = entry_empty_first->next;
            entry->value = value;
            ++claimed_count;
            return entry;
        }

    private:
        void ReleaseEntry(HeapEntry* entry, const std::unordered_map<Value::Type, std::function<void(HeapValue*)>>& deleters) {
#ifdef NLANG_DEBUG
            {
                HeapEntry* e = entry_empty_first;
                while (e) {
                    NLANG_ASSERT(e != entry);
                    e = e->next;
                }
            }
#endif
            NLANG_ASSERT(entry != nullptr);
            NLANG_ASSERT(entry >= begin() && entry < end());

            deleters.at(entry->value->type)(entry->value);
            entry->next = entry_empty_first;
            entry_empty_first = entry;
            --claimed_count;
        }

        HeapEntry* begin() const {
            return reinterpret_cast<HeapEntry*>(const_cast<PageHeader*>(this) + 1);
        }

        HeapEntry* end() const {
            return begin() + (Page::Size() - sizeof(PageHeader)) / sizeof(HeapEntry);
        }
    };

public:
    Heap() : page_first(nullptr), page_last(nullptr) {}

    Handle<HeapValue> StoreValue(HeapValue* value) {
        return Handle<HeapValue>((void*)ClaimEntry(value));
    }

    void RegisterDeleterForType(Value::Type type, const std::function<void(HeapValue*)>& deleter) {
        NLANG_ASSERT(deleters.find(type) == deleters.end());
        deleters[type] = deleter;
    }

private:
    PageHeader* AllocatePage() {
        auto p = Page::Allocate();
        auto page = new (p.data()) PageHeader();
        // insert free page to end
        InsertPage(page, page_last);
        return page;
    }

    void FreePage(PageHeader* page) {
        RemovePage(page);
        Page::Free(PageHandle(page));
    }

    void RemovePage(PageHeader* page) {
        NLANG_ASSERT(page != nullptr);

        auto prev = page->prev;
        auto next = page->next;

        if (prev) {
            prev->next = next;
        }

        if (next) {
            next->prev = prev;
        }

        if (page == page_first) {
            page_first = page->next;
        }

        if (page == page_last) {
            page_last = page->prev;
        }
    }

    void InsertPage(PageHeader* page, PageHeader* prev, PageHeader* next = nullptr) {
        NLANG_ASSERT(page != nullptr);

        if (prev) {
            next = prev->next;
        } else if (next) {
            prev = next->prev;
        }

        page->next = next;
        page->prev = prev;

        if (prev) {
            prev->next = page;
        }

        if (next) {
            next->prev = page;
        }

        if (next == page_first) {
            page_first = page;
        }

        if (prev == page_last) {
            page_last = page;
        }
    }

    HeapEntry* ClaimEntry(HeapValue* value) {
        PageHeader* page = page_last;
        if (!page || page->IsFull()) {
            page = AllocatePage();
        }
        HeapEntry* entry = page->ClaimEntry(value);
        if (page->IsFull()) {
            // move full page to begin
            RemovePage(page);
            InsertPage(page, nullptr, page_first);
        }
        return entry;
    }

private:
    // full pages first
    PageHeader* page_first;
    PageHeader* page_last;
    std::unordered_map<Value::Type, std::function<void(HeapValue*)>> deleters;
};

}

#endif //NLANG_HEAP_HPP
