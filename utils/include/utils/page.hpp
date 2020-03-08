#pragma once

#include "macro.hpp"

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <new>
#include <iterator>

#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
#include <unistd.h>
#include <sys/mman.h>
#include <new>
#include <stdexcept>
#elif defined(NLANG_PLATFORM_WINDOWS)
// Sets minimal API level requirement to Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <Windows.h>
#endif

namespace nlang {


class Page {
public:
    class PageIterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Page;
        using difference_type = std::ptrdiff_t;
        using pointer = Page*;
        using reference = Page&;

    public:
        PageIterator(void* data = nullptr) : data(data) {}
        PageIterator(const PageIterator&) = default;
        PageIterator& operator=(const PageIterator&) = default;

        ~PageIterator() = default;

        difference_type operator-(const PageIterator& other) const {
            return std::distance(static_cast<uint8_t*>(data), static_cast<uint8_t*>(other.data)) / Page::size();
        }

        PageIterator operator+(difference_type count) const {
            return PageIterator((char*)data + (difference_type)Page::size() * count);
        }

        PageIterator operator-(difference_type count) const {
            return PageIterator((char*)data - (difference_type)Page::size() * count);
        }

        PageIterator& operator+=(difference_type count) {
            data = (char*)data + (difference_type)Page::size() * count;
            return *this;
        }

        PageIterator& operator-=(difference_type count) {
            data = (char*)data - (difference_type)Page::size() * count;
            return *this;
        }

        PageIterator& operator++() {
            *this += 1;
            return *this;
        }

        PageIterator& operator--() {
            *this -= 1;
            return *this;
        }

        PageIterator operator++(int) {
            PageIterator p(data);
            *this += 1;
            return p;
        }

        PageIterator operator--(int) {
            PageIterator p(data);
            *this -= 1;
            return p;
        }

        bool operator==(const PageIterator& other) const {
            return data == other.data;
        }

        bool operator!=(const PageIterator& other) const {
            return data != other.data;
        }

        bool operator<(const PageIterator& other) const {
            return data < other.data;
        }

        bool operator>(const PageIterator& other) const {
            return data > other.data;
        }

        bool operator<=(const PageIterator& other) const {
            return data <= other.data;
        }

        bool operator>=(const PageIterator& other) const {
            return data >= other.data;
        }

        const Page& operator*() const {
            return *static_cast<const Page*>(data);
        }

        Page& operator*() {
            return *static_cast<Page*>(data);
        }

        Page* operator->() {
            return static_cast<Page*>(data);
        }

        const Page* operator->() const {
            return static_cast<const Page*>(data);
        }

    protected:
        void* data;
    };

    static std::pair<PageIterator, PageIterator> AllocateRange(size_t count = 1);
    static void FreeRange(const std::pair<PageIterator, PageIterator>& range);

    static Page* Allocate() {
        return &*AllocateRange().first;
    }

    static void Free(Page* page) {
        FreeRange(std::pair(PageIterator(page), PageIterator(static_cast<uint8_t*>(static_cast<void*>(page)) + size())));
    }

    Page(const Page&) = delete;
    Page(Page&&) = delete;
    Page& operator=(const Page&) = delete;
    Page& operator=(Page&&) = delete;

    void* data() const {
        return const_cast<Page*>(this);
    }

    static size_t size() {
        return size_;
    }

protected:
    Page() = default;
    ~Page() = default;

    static size_t SizeImpl() {
#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
        return sysconf(_SC_PAGESIZE);
#elif defined(NLANG_PLATFORM_WINDOWS)
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        return info.dwPageSize;
#else
        return 4096;
#endif
    }

    inline static size_t size_ = SizeImpl();
};




inline std::pair<Page::PageIterator, Page::PageIterator> Page::AllocateRange(size_t pages_count)  {
    void* raw_data;
    size_t final_size = size() * pages_count;
#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
    raw_data = mmap(nullptr, final_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw_data == MAP_FAILED) {
        throw std::bad_alloc();
    }
#elif defined(NLANG_PLATFORM_WINDOWS)
    raw_data = VirtualAlloc(nullptr, final_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!raw_data) {
        throw std::bad_alloc();
    }
#else
    raw_data = aligned_alloc(size(), final_size);
    if (!raw_data) {
        throw std::bad_alloc();
    }
#endif
    return std::pair(PageIterator(raw_data), PageIterator(raw_data) + pages_count);
}

inline void Page::FreeRange(const std::pair<PageIterator, PageIterator>& range) {
    void* raw_data = const_cast<Page*>(&*range.first);
    size_t final_size = size() * (range.second - range.first);
    if (!raw_data) {
        throw std::runtime_error("can't free nullptr page");
    }
#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
    munmap(raw_data, final_size);
#elif defined(NLANG_PLATFORM_WINDOWS)
    VirtualFree(raw_data, 0, MEM_RELEASE);
#else
    free(raw_data);
#endif
}

}
