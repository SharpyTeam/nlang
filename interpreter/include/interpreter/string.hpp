#pragma once

#include "value.hpp"
#include "stack_values.hpp"
#include "heap.hpp"

#include <utils/macro.hpp>

#include <cstring>
#include <tuple>
#include <codecvt>
#include <locale>

namespace nlang {

class String : public HeapValue {
public:
    static constexpr Value::Type TYPE = Value::Type::STRING;

    // Static methods

    template<typename ...StrArgs>
    static Handle <String> New(Heap& heap, const StrArgs& ...strings) {
        std::u32string s;
        if constexpr ((std::is_base_of_v<String, StrArgs> && ...)) {
            s = Concat(strings...);
        } else {
            s = ConvertAndConcat(strings...);
        }
        return heap.Store(new String(s)).As<String>();
    }

    String() = delete;

    [[nodiscard]] size_t GetHash() const {
        return hash;
    }

    [[nodiscard]] size_t GetLength() const {
        return data.length();
    }

    [[nodiscard]] const std::u32string& GetRawString() const {
        return data;
    }

    [[nodiscard]] Handle <Int32> GetCharCodeAt(const size_t index) const {
        return Int32::New(int32_t(data[index]));
    }

    [[nodiscard]] std::string AsStdString() const {
        typedef deletable_facet<std::codecvt<char32_t, char, std::mbstate_t>> facet_u32;
        std::wstring_convert<facet_u32, char32_t> conv;
        return conv.to_bytes(data);
    }

    String& operator=(const String&) = delete;

    String& operator=(String&&) = delete;

    bool operator==(const String& other) const {
        if (hash != other.hash)
            return false;

        // Even if hashes match, we still need to compare strings
        // to avoid wrong behaviour in case of collisions
        return this->data == other.data;
    }

    bool operator!=(const String& other) const {
        return !(*this == other);
    }

private:
    const std::u32string data;
    size_t hash;

    String(const std::u32string& string) : HeapValue(Type::STRING),
                                           data(string),
                                           hash(std::hash<std::u32string>{}(string)) {}

    String(std::u32string&& string) : HeapValue(Type::STRING),
                                      data(std::move(string)),
                                      hash(std::hash<std::u32string>{}(string)) {}

    template<typename Str>
    static std::u32string ConvertAndConcat(Str&& s) {
        typedef deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>> facet_u16;
        typedef deletable_facet<std::codecvt<char32_t, char, std::mbstate_t>> facet_u32;
        using StrType = std::decay_t<Str>;
        if constexpr (std::is_same_v<StrType, std::u32string>) {
            return s;
        } else if constexpr (std::is_same_v<StrType, std::u16string>) {
            std::wstring_convert<facet_u16, char16_t> u16_to_u8;
            std::string utf8 = u16_to_u8.to_bytes(s);
            std::wstring_convert<facet_u32, char32_t> u8_to_u32;
            return u8_to_u32.from_bytes(utf8);
        } else if constexpr (std::is_same_v<StrType, std::string>) {
            std::wstring_convert<facet_u32, char32_t> u8_to_u32;
            return u8_to_u32.from_bytes(s);
        } else if constexpr (std::is_same_v<StrType, String>) {
            return s.GetRawString();
        } else if constexpr (std::is_same_v<StrType, const char32_t*>) {
            return std::u32string(s);
        } else if constexpr (std::is_same_v<StrType, const char16_t*>) {
            std::u16string u16s(s);
            std::wstring_convert<facet_u16, char16_t> u16_to_u8;
            std::string utf8 = u16_to_u8.to_bytes(s);
            std::wstring_convert<facet_u32, char32_t> u8_to_u32;
            return u8_to_u32.from_bytes(utf8);
        } else if constexpr (std::is_same_v<StrType, const char*>) {
            std::wstring_convert<facet_u32, char32_t> u8_to_u32;
            return u8_to_u32.from_bytes(s);
        } else {
            static_assert(dependent_false<StrType>::value,
                          "invalid template argument (string) type:"
                          " must be std::string, std::u16string, std::u32string, String or char*");
        }
    }

    template<typename F, typename S, typename ...StrTail>
    static std::u32string ConvertAndConcat(F&& f, S&& s, StrTail&& ...strings) {
        return ConvertAndConcat<F>(std::forward<F>(f)) +
               ConvertAndConcat<S, StrTail...>(std::forward<S>(s), std::forward<StrTail>(strings)...);
    }

private:
    template<typename Str>
    static std::u32string Concat(Str&& s) {
        static_assert(std::is_base_of_v<Str, String>, "all arguments must be nlang::String classes");
        return s.GetRawString();
    }

    template<typename F, typename S, typename ...StrTail>
    static std::u32string Concat(F&& f, S&& s, StrTail&& ...strings) {
        return ConvertAndConcat<F>(std::forward<F>(f)) +
               ConvertAndConcat<S, StrTail...>(std::forward<S>(s), std::forward<StrTail>(strings)...);
    }

};

}

namespace std {

template<>
struct hash<nlang::String> {
    size_t operator()(const nlang::String& s) const {
        return s.GetHash();
    }
};

template<>
struct hash<nlang::Handle<nlang::String>> {
    size_t operator()(const nlang::Handle<nlang::String>& h) const {
        return h->GetHash();
    }
};

template<>
struct equal_to<nlang::Handle<nlang::String>> {
    bool operator()(const nlang::Handle<nlang::String>& lhs, const nlang::Handle<nlang::String>& rhs) const {
        return *lhs == *rhs;
    }
};

}
