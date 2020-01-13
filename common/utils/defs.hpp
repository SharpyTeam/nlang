//
// Created by ayles on 1/10/20.
//

#ifndef NLANG_DEFS_HPP
#define NLANG_DEFS_HPP

#if defined(__clang__)
#define NLANG_COMPILER_CLANG
#define NLANG_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(__GNUC__)
#define NLANG_COMPILER_GCC
#define NLANG_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define NLANG_COMPILER_MSVC
#define NLANG_FORCE_INLINE inline __forceinline
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
