#pragma once

#include <common/handles/handle.hpp>
#include <common/values/primitives.hpp>
#include <common/heap/heap.hpp>

#include <utils/macro.hpp>

#include <unicode/unistr.h>

#include <cstring>
#include <tuple>
#include <codecvt>
#include <locale>

namespace nlang {

class String : public HeapValue {
public:
    String() = delete;
    String& operator=(const String&) = delete;
    String& operator=(String&&) = delete;

    virtual ~String() override = default;


    [[nodiscard]] size_t GetHash() const {
        return hash;
    }

    [[nodiscard]] size_t GetLength() const {
        return data.length();
    }

    [[nodiscard]] const icu::UnicodeString& GetRawString() const {
        return data;
    }

    [[nodiscard]] Handle<Int32> GetCharCodeAt(const size_t index) const {
        return Int32::New((int32_t)data.char32At(index));
    }

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

    template<typename ...StrArgs>
    static Handle<String> New(Heap* heap, const StrArgs& ...strings) {
        icu::UnicodeString s;
        if constexpr ((std::is_base_of_v<String, StrArgs> && ...)) {
            s = Concat(strings...);
        } else {
            s = ConvertAndConcat(strings...);
        }
        return heap->Store(new String(s)).As<String>();
    }

    static Handle<String> NewFromCharCode(Heap* heap, int32_t char_code) {
        icu::UnicodeString s = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(&char_code), 1);
        return heap->Store(new String(s)).As<String>();
    }

private:
    icu::UnicodeString data;
    size_t hash;

    String(const icu::UnicodeString& string)
        : data(string)
        , hash(data.hashCode())
    {}

    String(icu::UnicodeString&& string)
        : data(std::move(string))
        , hash(data.hashCode())
    {}

    template<typename Str>
    static icu::UnicodeString ConvertAndConcat(Str&& s) {
        using StrType = std::decay_t<Str>;
        if constexpr (std::is_same_v<StrType, icu::UnicodeString>) {
            return s;
        } else if constexpr (std::is_same_v<StrType, std::u32string>) {
            return icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(s.data()), s.size());
        } else if constexpr (std::is_same_v<StrType, std::u16string>) {
            return icu::UnicodeString(s.data(), s.size());
        } else if constexpr (std::is_same_v<StrType, std::string>) {
            return icu::UnicodeString::fromUTF8(std::forward<Str>(s));
        } else if constexpr (std::is_same_v<StrType, String>) {
            return s.GetRawString();
        } else if constexpr (std::is_same_v<StrType, const char32_t*>) {
            return icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(s), std::char_traits<char32_t>::length(s));
        } else if constexpr (std::is_same_v<StrType, const char16_t*>) {
            return icu::UnicodeString(s, std::char_traits<char16_t>::length(s));
        } else if constexpr (std::is_same_v<StrType, const char*>) {
            return icu::UnicodeString::fromUTF8(s);
        } else {
            static_assert(dependent_false<StrType>::value,
                          "invalid template argument (string) type:"
                          " must be std::string, std::u16string, std::u32string, String or char*");
        }
    }

    template<typename F, typename S, typename ...StrTail>
    static icu::UnicodeString ConvertAndConcat(F&& f, S&& s, StrTail&& ...strings) {
        return ConvertAndConcat<F>(std::forward<F>(f)) +
               ConvertAndConcat<S, StrTail...>(std::forward<S>(s), std::forward<StrTail>(strings)...);
    }

private:
    template<typename Str>
    static icu::UnicodeString Concat(Str&& s) {
        static_assert(std::is_base_of_v<Str, String>, "all arguments must be nlang::String classes");
        return s.GetRawString();
    }

    template<typename F, typename S, typename ...StrTail>
    static icu::UnicodeString Concat(F&& f, S&& s, StrTail&& ...strings) {
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
