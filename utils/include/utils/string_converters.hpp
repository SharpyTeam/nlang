#pragma once

#include "defs.hpp"
#include "traits.hpp"

#include <string>
#include <locale>
#include <codecvt>

#ifdef NLANG_COMPILER_MSVC
    #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#if defined(NLANG_COMPILER_MSVC) && _MSC_VER < 1922

// Workaround for old MSVC,
// which doesn't provide all std::codecvt facades
// (till Visual Studio 2019 version 16.2)

#pragma message ( "Using a workaround!" )

std::string utf16_to_utf8(const std::u16string &s) {
    std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
    auto p = reinterpret_cast<const int16_t *>(s.data());
    return convert.to_bytes(p, p + s.size());
}

std::string utf32_to_utf8(const std::u32string &s) {
    std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> convert;
    auto p = reinterpret_cast<const int32_t *>(s.data());
    return convert.to_bytes(p, p + s.size());
}

std::u32string utf8_to_utf32(const std::string &s) {
    std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> convert;
    return reinterpret_cast<const char32_t *>(convert.from_bytes(s).data());
}

#else

std::string utf16_to_utf8(const std::u16string &s) {
    typedef deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>> facet_u16;
    std::wstring_convert<facet_u16, char16_t> u16_to_u8;
    return u16_to_u8.to_bytes(s);
}

std::string utf32_to_utf8(const std::u32string &s) {
    typedef deletable_facet<std::codecvt<char32_t, char, std::mbstate_t>> facet_u32;
    std::wstring_convert<facet_u32, char32_t> u32_to_u8;
    return u32_to_u8.to_bytes(s);
}

std::u32string utf8_to_utf32(const std::string &s) {
    typedef deletable_facet<std::codecvt<char32_t, char, std::mbstate_t>> facet_u32;
    std::wstring_convert<facet_u32, char32_t> u8_to_u32;
    return u8_to_u32.from_bytes(s);
}

#endif