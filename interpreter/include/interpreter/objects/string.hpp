#pragma once

#include <interpreter/handle.hpp>
#include <interpreter/objects/primitives.hpp>
#include <interpreter/heap.hpp>

#include <utils/macro.hpp>
#include <utils/strings.hpp>

#include <cstring>
#include <tuple>
#include <codecvt>
#include <locale>

namespace nlang {

class String : public HeapValue, public UString {
public:
    String() = delete;
    String& operator=(const String&) = delete;
    String& operator=(String&&) = delete;

    virtual ~String() override = default;

    template<typename ...StrArgs>
    static Handle<String> New(Heap* heap, const StrArgs& ...strings) {
        UString s;
        if constexpr ((std::is_base_of_v<String, StrArgs> && ...)) {
            s = Concat(strings...);
        } else {
            s = ConvertAndConcat(strings...);
        }
        return heap->Store(new String(s)).As<String>();
    }

    static Handle<String> NewFromChar(Heap* heap, Char c) {
        return heap->Store(new String(UString() + c)).As<String>();
    }

private:
    String(const UString& string)
        : UString(string)
    {}

    String(UString&& string)
        : UString(std::move(string))
    {}

    template<typename Str>
    static UString ConvertAndConcat(Str&& s) {
        using StrType = std::decay_t<Str>;
        if constexpr (std::is_same_v<StrType, UString>) {
            return s;
        } else if constexpr (std::is_same_v<StrType, std::u32string>) {
            return icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(s.data()), s.size());
        } else if constexpr (std::is_same_v<StrType, std::u16string>) {
            return icu::UnicodeString(s.data(), s.size());
        } else if constexpr (std::is_same_v<StrType, std::string>) {
            return icu::UnicodeString::fromUTF8(std::forward<Str>(s));
        } else if constexpr (std::is_same_v<StrType, String>) {
            return s;
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
    static UString ConvertAndConcat(F&& f, S&& s, StrTail&& ...strings) {
        return ConvertAndConcat<F>(std::forward<F>(f)) +
               ConvertAndConcat<S, StrTail...>(std::forward<S>(s), std::forward<StrTail>(strings)...);
    }

    void ForEachReference(std::function<void(Handle<Value>)> handler) override {}

private:
    template<typename Str>
    static UString Concat(Str&& s) {
        static_assert(std::is_base_of_v<Str, String>, "all arguments must be nlang::String classes");
        return s;
    }

    template<typename F, typename S, typename ...StrTail>
    static UString Concat(F&& f, S&& s, StrTail&& ...strings) {
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
