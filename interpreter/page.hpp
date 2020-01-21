//
// Created by ayles on 1/15/20.
//

#ifndef NLANG_PAGE_HPP
#define NLANG_PAGE_HPP

#include <utils/defs.hpp>

#include <cstddef>
#include <type_traits>

#if defined(NLANG_PLATFORM_LINUX)
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

template<typename T>
class TypedPageHandle;

class PageHandle {
    friend class Page;

public:
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

    template<typename T>
    TypedPageHandle<T> Typed() const;

protected:
    PageHandle(void* data_) : data_(data_) {}

    void* data_;
};

class Page {
public:
    static PageRange AllocateRange(size_t pages_count);
    static PageHandle Allocate();
    static void FreeRange(PageRange range);
    static void Free(PageHandle handle);

    static size_t Distance(PageHandle a, PageHandle b) {
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

    static size_t Size() {
        static size_t size = SizeImpl();
        return size;
    }

protected:
    Page() = delete;

    static size_t SizeImpl() {
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

template<typename T>
class TypedPageHandle : public PageHandle {
    friend class PageHandle;

public:
    TypedPageHandle(const TypedPageHandle&) = default;
    TypedPageHandle& operator=(const TypedPageHandle&) = default;

    ~TypedPageHandle() = default;

    size_t size() const {
        return size_;
    }

    T* data() const {
        return static_cast<T*>(data_);
    }

    T* begin() const {
        return static_cast<T*>(data_);
    }

    T* end() const {
        return static_cast<T*>(data_) + size_;
    }

    T& operator[](size_t index) {
        return static_cast<T*>(data_)[index];
    }

    const T& operator[](size_t index) const {
        return static_cast<T*>(data_)[index];
    }

    TypedPageHandle operator+(intptr_t count) const {
        return TypedPageHandle((void*)((size_t)PageHandle::data_ + (intptr_t)PageHandle::size() * count));
    }

    TypedPageHandle operator-(intptr_t count) const {
        return TypedPageHandle((void*)((size_t)PageHandle::data_ - (intptr_t)PageHandle::size() * count));
    }

    TypedPageHandle& operator+=(intptr_t count) {
        data_ = (void*)((size_t)PageHandle::data_ + (intptr_t)PageHandle::size() * count);
        return *this;
    }

    TypedPageHandle& operator-=(intptr_t count) {
        data_ = (void*)((size_t)PageHandle::data_ - (intptr_t)PageHandle::size() * count);
        return *this;
    }

    TypedPageHandle& operator++() {
        *this += 1;
        return *this;
    }

    TypedPageHandle& operator--() {
        *this -= 1;
        return *this;
    }

    TypedPageHandle operator++(int) {
        TypedPageHandle p(data_);
        *this += 1;
        return p;
    }

    TypedPageHandle operator--(int) {
        TypedPageHandle p(data_);
        *this -= 1;
        return p;
    }

    const TypedPageHandle& operator*() const {
        return *this;
    }

    TypedPageHandle& operator*() {
        return *this;
    }

private:
    TypedPageHandle(void* data_) : PageHandle(data_) {}

    static inline size_t size_ = Page::Size() / sizeof(T);
};

size_t PageHandle::size() const {
    return Page::Size();
}

template<typename T>
TypedPageHandle<T> PageHandle::Typed() const {
    return TypedPageHandle<T>(data_);
}

template<typename T>
class TypedPageRange;

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

    template<typename T>
    TypedPageRange<T> Typed() const;

private:
    PageHandle begin_;
    PageHandle end_;
};

template<typename T>
class TypedPageRange {
public:
    TypedPageRange(TypedPageHandle<T> a, TypedPageHandle<T> b) : begin_(a), end_(b) {}

    TypedPageHandle<T> begin() const {
        return begin_;
    }

    TypedPageHandle<T> end() const {
        return end_;
    }

    size_t size() const {
        return Page::Distance(begin_, end_);
    }

private:
    TypedPageHandle<T> begin_;
    TypedPageHandle<T> end_;
};


template<typename T>
TypedPageRange<T> PageRange::Typed() const {
    return TypedPageRange<T>(begin_.Typed<T>(), end_.Typed<T>());
}


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

#endif //NLANG_PAGE_HPP
