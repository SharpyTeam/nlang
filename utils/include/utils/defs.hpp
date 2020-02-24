#pragma once


#ifdef NDEBUG
#define NLANG_RELEASE
#else
#define NLANG_DEBUG
#endif


#ifdef NLANG_DEBUG

#include "backtrace.hpp"

#include <cstdlib>

#define NLANG_ASSERT(...)                       \
{                                               \
    if (!static_cast<bool>(__VA_ARGS__)) {      \
        fprintf(stderr, "Assertion \"%s\" failed\n", #__VA_ARGS__); \
        nlang::utils::PrintBacktrace();         \
        exit(0);                                \
    }                                           \
}                                               \
do {} while (0)

#else
#define NLANG_ASSERT(...) do {} while (0)
#endif

#if defined(__clang__)
#define NLANG_COMPILER_CLANG
#define NLANG_FORCE_INLINE inline __attribute__((always_inline))
#define NLANG_LIKELY(x)      __builtin_expect(!!(x), 1)
#define NLANG_UNLIKELY(x)    __builtin_expect(!!(x), 0)
#elif defined(__GNUC__)
#define NLANG_COMPILER_GCC
#define NLANG_FORCE_INLINE inline __attribute__((always_inline))
#define NLANG_LIKELY(x)      __builtin_expect(!!(x), 1)
#define NLANG_UNLIKELY(x)    __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
#define NLANG_COMPILER_MSVC
#define NLANG_FORCE_INLINE inline __forceinline
#define NLANG_LIKELY(x)      x
#define NLANG_UNLIKELY(x)    x
#endif

#if defined(__APPLE__)
#define NLANG_PLATFORM_MACOS
#elif defined(_WIN32)
#define NLANG_PLATFORM_WINDOWS
#elif defined(__ANDROID__)
#define NLANG_PLATFORM_ANDROID
#elif defined(__linux__)
#define NLANG_PLATFORM_LINUX
#endif
