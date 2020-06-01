/**
 * Provide mappings to OS page allocation calls
 */

#pragma once

#include <utils/macro.hpp>

#include <cstddef>
#include <memory>

#if defined(NLANG_PLATFORM_WINDOWS) && defined(NLANG_COMPILER_MSVC)
#include <malloc.h>
#endif

namespace nlang {

NLANG_FORCE_INLINE void* AlignedAlloc(std::size_t alignment, std::size_t size) {
#if defined(NLANG_PLATFORM_WINDOWS) && defined(NLANG_COMPILER_MSVC)
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
#endif
}

NLANG_FORCE_INLINE void AlignedFree(void* ptr) {
#if defined(NLANG_PLATFORM_WINDOWS) && defined(NLANG_COMPILER_MSVC)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

}