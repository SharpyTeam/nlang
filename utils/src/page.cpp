#include <utils/alloc/page.hpp>

#include <stdexcept>
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

inline std::pair<Page::PageIterator, Page::PageIterator> Page::AllocateRange(size_t pages_count)  {
    void* raw_data = nullptr;
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

size_t Page::SizeImpl() {
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

Page* Page::Allocate() {
    return &*AllocateRange().first;
}

void Page::Free(Page* page) {
    FreeRange(std::pair(PageIterator(page), PageIterator(static_cast<uint8_t*>(static_cast<void*>(page)) + size())));
}

}