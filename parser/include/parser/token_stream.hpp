#pragma once

#include <common/token.hpp>

#include <utils/macro.hpp>
#include <utils/pointers/unique_ptr.hpp>
#include <utils/strings.hpp>

#include <unicode/regex.h>

#include <iterator>
#include <memory>
#include <vector>
#include <regex>
#include <optional>


namespace nlang {

class Heap;

class TokenStream final {
public:
    TokenStream(const TokenStream&) = delete;
    TokenStream(TokenStream&&) = delete;
    TokenStream& operator=(const TokenStream&) = delete;
    TokenStream& operator=(TokenStream&&) = delete;

    bool HasNext();
    TokenInstance Next();

    static UniquePtr<TokenStream> New(UString&& source) {
        return UniquePtr<TokenStream>(new TokenStream(std::move(source)));
    }

    static UniquePtr<TokenStream> New(const UString& source) {
        return UniquePtr<TokenStream>(new TokenStream(source));
    }

private:
    TokenStream(const UString& source);
    TokenStream(UString&& source);

private:
    UString source;
    UString source_view;
    int32_t pos;
    int32_t row;
    int32_t col;
    UErrorCode error_code = U_ZERO_ERROR;
    std::vector<std::pair<UniquePtr<icu::RegexMatcher>, Token>> regex_tokens;

    static constexpr uint32_t regex_flags = 0;
};

}