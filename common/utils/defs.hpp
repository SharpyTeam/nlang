//
// Created by ayles on 1/10/20.
//

#ifndef NLANG_DEFS_HPP
#define NLANG_DEFS_HPP


#ifdef NDEBUG
#define NLANG_RELEASE
#else
#define NLANG_DEBUG
#endif


#ifdef NLANG_DEBUG

#include <backward.hpp>
#include <cstdlib>
#include <iostream>

#define NLANG_ASSERT(...)                       \
{                                               \
    if (!static_cast<bool>(__VA_ARGS__)) {      \
        fprintf(stderr, "Assertion \"%s\" failed\n", #__VA_ARGS__); \
        using namespace backward;               \
        StackTrace st; st.load_here(32);        \
        Printer p;                              \
        p.object = true;                        \
        p.color_mode = ColorMode::always;       \
        p.address = true;                       \
        p.print(st, stderr);                    \
        exit(1);                                \
    }                                           \
}

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

namespace nlang::defs {

enum class Compiler {
    MSVC,
    CLANG,
    GCC,
    UNKNOWN
};

enum class Platform {
    WINDOWS,
    LINUX,
    MACOS,
    UNKNOWN
};

inline constexpr Compiler compiler =
#if defined(NLANG_COMPILER_CLANG)
Compiler::CLANG
#elif defined(NLANG_COMPILER_GCC)
Compiler::GCC
#elif defined(NLANG_COMPILER_MSVC)
Compiler::MSVC
#else
Compiler::UNKNOWN
#endif
;

inline constexpr Platform platform =
#if defined(NLANG_PLATFORM_APPLE)
Platform::MACOS
#elif defined(NLANG_PLATFORM_WINDOWS)
Platform::WINDOWS
#elif defined(NLANG_PLATFORM_ANDROID)
Platform::ANDROID
#elif defined(NLANG_PLATFORM_LINUX)
Platform::LINUX
#else
Platform::UNKNOWN
#endif
;

}

#endif //NLANG_DEFS_HPP
