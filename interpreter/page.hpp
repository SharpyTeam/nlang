//
// Created by ayles on 1/15/20.
//

#ifndef NLANG_PAGE_HPP
#define NLANG_PAGE_HPP

#include <utils/defs.hpp>
#include <cstddef>

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

    PageHandle Next() const {
        return PageHandle((void*)((size_t)data_ + size()));
    }

    PageHandle Prev() const {
        return PageHandle((void*)((size_t)data_ - size()));
    }

    template<typename T>
    TypedPageHandle<T> Typed() const;

protected:
    PageHandle(void* data_) : data_(data_) {}

    void* data_;
};

class Page {
public:
    static PageHandle AllocateContiguous(size_t pages_count) {
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
        return PageHandle(raw_data);
    }

    static PageHandle Allocate() {
        return AllocateContiguous(1);
    }

    static void FreeContiguous(PageHandle handle, size_t pages_count) {
        void* raw_data = handle.data_;
        size_t size = Size() * pages_count;
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

    static void Free(PageHandle handle) {
        FreeContiguous(handle, 1);
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

    TypedPageHandle Next() const {
        return TypedPageHandle((void*)((size_t)PageHandle::data_ + PageHandle::size()));
    }

    TypedPageHandle Prev() const {
        return TypedPageHandle((void*)((size_t)PageHandle::data_ - PageHandle::size()));
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

}

#endif //NLANG_PAGE_HPP
