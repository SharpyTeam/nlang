#pragma once

#include <cstddef>
#include <cstdlib>
#include "macro.hpp"

#if defined(NLANG_PLATFORM_WINDOWS) && defined(NLANG_COMPILER_MSVC)
#include "malloc.h"
#endif

inline void* do_aligned_alloc(std::size_t alignment, std::size_t size) {
#if defined(NLANG_PLATFORM_WINDOWS) && defined(NLANG_COMPILER_MSVC)
    return _aligned_malloc(size, alignment);
#else
    return std::aligned_alloc(alignment, size);
#endif
}