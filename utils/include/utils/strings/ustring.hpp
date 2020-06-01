#pragma once

#include <unicode/unistr.h>

#include <string>
#include <iostream>

namespace nlang {

using Char = int32_t;

/**
 * An extension of ICU's UnicodeString
 * Stores a low-level strings.
 */
class UString : public icu::UnicodeString {
public:
    UString() = default;

    UString(const char* text)
        : icu::UnicodeString(icu::UnicodeString::fromUTF8(text))
    {}

    UString(const std::string& text)
        : icu::UnicodeString(icu::UnicodeString::fromUTF8(text))
    {}

    UString(const UString& s, int32_t start)
        : icu::UnicodeString(s, start)
    {}

    UString(const UString& s, int32_t start, int32_t length)
        : icu::UnicodeString(s, start, length)
    {}

    // TODO make private
    UString(const icu::UnicodeString& s)
        : icu::UnicodeString(s)
    {}

    // TODO make private
    UString(icu::UnicodeString&& s)
        : icu::UnicodeString(std::move(s))
    {}

    UString(const UString&) = default;
    UString(UString&&) = default;
    UString& operator=(const UString&) = default;
    UString& operator=(UString&&) = default;

    virtual ~UString() = default;


    int32_t GetLength() const {
        return getLength();
    }

    Char GetCharAt(int32_t offset) const {
        return getChar32At(offset);
    }

    void SetCharAt(int32_t offset, Char c) {
        replace(offset, 1, c);
    }

    int32_t GetHash() const {
        return hashCode();
    }

    std::string GetStdStr() const {
        std::string s;
        toUTF8String(s);
        return s;
    }

    UString SubStringView(int32_t start = 0, int32_t length = INT32_MAX) const {
        return UString(tempSubString(start, length));
    }

    UString SubString(int32_t start) const {
        return UString(*this, start);
    }

    UString SubString(int32_t start, int32_t length) const {
        return UString(*this, start, length);
    }

    bool IsEmpty() const {
        return isEmpty();
    }

    void Clear() {
        remove();
    }

    UString Unescape() const {
        return unescape();
    }

    UString& Trim() {
        trim();
        return *this;
    }

    int32_t IndexOf(Char c) const {
        return indexOf(c);
    }

    int32_t IndexOf(Char c, int32_t start) const {
        return indexOf(c, start);
    }

    int32_t IndexOf(Char c, int32_t start, int32_t length) const {
        return indexOf(c, start, length);
    }

    int32_t IndexOf(const UString& s) const {
        return indexOf(s);
    }

    int32_t IndexOf(const UString& s, int32_t start) const {
        return indexOf(s, start);
    }

    int32_t IndexOf(const UString& s, int32_t start, int32_t length) const {
        return indexOf(s, start, length);
    }

    int32_t LastIndexOf(Char c) const {
        return lastIndexOf(c);
    }

    int32_t LastIndexOf(Char c, int32_t start) const {
        return lastIndexOf(c, start);
    }

    int32_t LastIndexOf(Char c, int32_t start, int32_t length) const {
        return lastIndexOf(c, start, length);
    }

    int32_t LastIndexOf(const UString& s) const {
        return lastIndexOf(s);
    }

    int32_t LastIndexOf(const UString& s, int32_t start) const {
        return lastIndexOf(s, start);
    }

    int32_t LastIndexOf(const UString& s, int32_t start, int32_t length) const {
        return lastIndexOf(s, start, length);
    }
    
    bool Truncate(int32_t length) {
        return truncate(length);
    }

    bool operator==(const UString& other) const {
        return icu::UnicodeString::operator==(other);
    }

    bool operator!=(const UString& other) const {
        return icu::UnicodeString::operator!=(other);
    }

    bool operator<(const UString& other) const {
        return icu::UnicodeString::operator<(other);
    }

    bool operator>(const UString& other) const {
        return icu::UnicodeString::operator>(other);
    }

    bool operator<=(const UString& other) const {
        return icu::UnicodeString::operator<=(other);
    }

    bool operator>=(const UString& other) const {
        return icu::UnicodeString::operator>=(other);
    }

    explicit operator bool() const {
        return !IsEmpty();
    }

    UString& operator+=(const UString& other) {
        icu::UnicodeString::operator+=(other);
        return *this;
    }

    UString& operator+=(Char c) {
        icu::UnicodeString::operator+=(c);
        return *this;
    }

    UString operator+(const UString& other) const {
        return UString(std::move((icu::UnicodeString(getLength() + other.getLength()) += *this) += other));
    }

private:

};


inline std::ostream& operator<<(std::ostream& os, const UString& s) {
    os << s.GetStdStr();
    return os;
}

inline std::istream& operator>>(std::istream& is, UString& s) {
    std::string tmp;
    is >> tmp;
    s = UString(tmp);
    return is;
}


}


template<>
struct std::hash<nlang::UString> {
    size_t operator()(const nlang::UString& s) const {
        return s.GetHash();
    }
};