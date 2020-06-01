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

/**
 * Token stream
 * Actually parses the string, looking for tokens
 */
class TokenStream final {
public:
    TokenStream(const TokenStream&) = delete;
    TokenStream(TokenStream&&) = delete;
    TokenStream& operator=(const TokenStream&) = delete;
    TokenStream& operator=(TokenStream&&) = delete;

    /**
     * Checks if end was reached
     * @return
     */
    bool HasNext();
    /**
     * Tries to find next token in string
     * @return Next token instance
     */
    TokenInstance Next();
    static UniquePtr<TokenStream> New(UString&& source) {
        return UniquePtr<TokenStream>(new TokenStream(std::move(source)));
    }
    /**
     * Creates new token stream instance from source string
     * @param source Source string
     * @return Unique pointer to created token stream
     */
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