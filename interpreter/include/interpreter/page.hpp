#pragma once

#include <utils/defs.hpp>

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <new>

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

class PageRange;

class PageHandle {
    friend class Page;

public:
    PageHandle() : data_(nullptr) {}
    PageHandle(void* data_) : data_(data_) {}
    PageHandle(const PageHandle&) = default;
    PageHandle& operator=(const PageHandle&) = default;

    ~PageHandle() = default;

    size_t size() const;

    void* data() const {
        return data_;
    }

    PageHandle operator+(intptr_t count) const {
        return PageHandle((void*)((size_t)data_ + (intptr_t)size() * count));
    }

    PageHandle operator-(intptr_t count) const {
        return PageHandle((void*)((size_t)data_ - (intptr_t)size() * count));
    }

    PageHandle& operator+=(intptr_t count) {
        data_ = (void*)((size_t)data_ + size() * count);
        return *this;
    }

    PageHandle& operator-=(intptr_t count) {
        data_ = (void*)((size_t)data_ - size() * count);
        return *this;
    }

    PageHandle& operator++() {
        *this += 1;
        return *this;
    }

    PageHandle& operator--() {
        *this -= 1;
        return *this;
    }

    PageHandle operator++(int) {
        PageHandle p(data_);
        *this += 1;
        return p;
    }

    PageHandle operator--(int) {
        PageHandle p(data_);
        *this -= 1;
        return p;
    }

    bool operator==(const PageHandle& other) const {
        return data_ == other.data_;
    }

    bool operator!=(const PageHandle& other) const {
        return data_ != other.data_;
    }

    bool operator<(const PageHandle& other) const {
        return data_ < other.data_;
    }

    bool operator>(const PageHandle& other) const {
        return data_ > other.data_;
    }

    bool operator<=(const PageHandle& other) const {
        return data_ <= other.data_;
    }

    bool operator>=(const PageHandle& other) const {
        return data_ >= other.data_;
    }

    const PageHandle& operator*() const {
        return *this;
    }

    PageHandle& operator*() {
        return *this;
    }

protected:
    void* data_;
};


class Page {
public:
    inline static PageRange AllocateRange(size_t pages_count);
    inline static PageHandle Allocate();
    inline static void FreeRange(PageRange range);
    inline static void Free(PageHandle handle);

    inline static size_t Distance(PageHandle a, PageHandle b) {
        if (a > b) {
            return ((size_t)a.data_ - (size_t)b.data_) / Size();
        } else {
            return ((size_t)b.data_ - (size_t)a.data_) / Size();
        }
    }

    Page(const Page&) = delete;
    Page(Page&&) = delete;
    Page& operator=(const Page&) = delete;
    Page& operator=(Page&&) = delete;
    ~Page() = delete;

    inline static size_t Size() {
        static size_t size = SizeImpl();
        return size;
    }

protected:
    Page() = delete;

    inline static size_t SizeImpl() {
#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
        return getpagesize();
#elif defined(NLANG_PLATFORM_WINDOWS)
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        return info.dwPageSize;
#else
        return 4096;
#endif
    }
};

inline size_t PageHandle::size() const {
    return Page::Size();
}


class PageRange {
public:
    PageRange(PageHandle a, PageHandle b) : begin_(a), end_(b) {}

    PageHandle begin() const {
        return begin_;
    }

    PageHandle end() const {
        return end_;
    }

    size_t size() const {
        return Page::Distance(begin_, end_);
    }

private:
    PageHandle begin_;
    PageHandle end_;
};


PageRange Page::AllocateRange(size_t pages_count)  {
    void* raw_data;
    size_t size = Size() * pages_count;
#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
    raw_data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw_data == MAP_FAILED) {
        throw std::bad_alloc();
    }
#elif defined(NLANG_PLATFORM_WINDOWS)
    raw_data = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!raw_data) {
        throw std::bad_alloc();
    }
#else
    raw_data = aligned_alloc(Size(), size);
    if (!raw_data) {
        throw std::bad_alloc();
    }
#endif
    return PageRange(PageHandle(raw_data), PageHandle(raw_data) + pages_count);
}

PageHandle Page::Allocate() {
    return AllocateRange(1).begin();
}

void Page::FreeRange(PageRange range) {
    void* raw_data = range.begin().data_;
    size_t size = Size() * range.size();
    if (!raw_data) {
        throw std::runtime_error("can't free nullptr page");
    }
#if defined(NLANG_PLATFORM_LINUX) || defined(NLANG_PLATFORM_MACOS)
    munmap(raw_data, size);
#elif defined(NLANG_PLATFORM_WINDOWS)
    VirtualFree(raw_data, 0, MEM_RELEASE);
#else
    free(raw_data);
#endif
}

void Page::Free(PageHandle handle) {
    FreeRange(PageRange(handle, handle + 1));
}

}
