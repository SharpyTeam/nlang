#pragma once

#include <common/token.hpp>
#include <common/heap/heap.hpp>
#include <common/handles/handle.hpp>
#include <common/objects/string.hpp>

#include <utils/macro.hpp>
#include <utils/holder.hpp>

#include <unicode/regex.h>

#include <iterator>
#include <memory>
#include <vector>
#include <regex>
#include <optional>


namespace nlang {

class TokenStream final {
public:
    TokenStream(const TokenStream&) = delete;
    TokenStream(TokenStream&&) = delete;
    TokenStream& operator=(const TokenStream&) = delete;
    TokenStream& operator=(TokenStream&&) = delete;

    bool HasNext() {
        return pos != (size_t)-1;
    }

    TokenInstance Next() {
        NLANG_ASSERT(HasNext());

        while (pos != source->GetRawString().length()) {
            for (auto& [regex, token] : regex_tokens) {
                regex->reset(source_view);
                if (regex->find(error_code)) {
                    icu::UnicodeString str = regex->group(error_code);

                    Token actual_token = token;
                    if (token == Token::OPERATOR_OR_PUNCTUATION || token == Token::IDENTIFIER) {
                        try {
                            std::string s;
                            str.toUTF8String(s);
                            actual_token = TokenUtils::GetTokenByText(s);
                        } catch (std::out_of_range&) {}
                    }

                    const size_t saved_row = row;
                    const size_t saved_column = col;

                    for (size_t i = 0; i < str.length(); ++i) {
                        ++col;
                        if (str.char32At(i) == '\n') {
                            row++;
                            col = 1;
                        }
                    }

                    const size_t pos_in_string = pos;
                    pos += str.length();
                    source_view = source->GetRawString().tempSubString(pos);
                    size_t len = str.length();
                    if (actual_token == Token::STRING) {
                        str = icu::UnicodeString(str, 1, str.length() - 2);
                    }
                    return TokenInstance { actual_token, pos_in_string, len, saved_row, saved_column, String::New(heap, str) };
                }
            }
        }

        pos = -1;
        return TokenInstance { Token::THE_EOF, pos, 0, row, col, Handle<String>() };
    }

    static Holder<TokenStream> New(Heap* heap, Handle<String> source) {
        return Holder<TokenStream>(new TokenStream(heap, source));
    }

private:
    TokenStream(Heap* heap, Handle<String> source)
        : heap(heap)
        , source(source)
        , source_view(source->GetRawString().tempSubString())
        , pos(0)
        , row(1)
        , col(1)
    {
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^[ \t\r]+)", regex_flags, error_code), Token::SPACE);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^\/\/.*?(\n|$))", regex_flags, error_code), Token::COMMENT);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^\/\*.*?\*\/)", regex_flags, error_code), Token::COMMENT);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^((\+\+|\-\-|\+=|-=|\*=|\/=|\%=|==|!=|>=|<=|<<|>>)|\(|\)|\{|\}|\[|\]|;|:|,|\.|=|\*|\/|\+|\-|!|>|<|\~|&|\||\^))", regex_flags, error_code), Token::OPERATOR_OR_PUNCTUATION);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^([^\x00-\x7F]|[a-zA-Z_])([^\x00-\x7F]|[a-zA-Z0-9_])*)", regex_flags, error_code), Token::IDENTIFIER);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^"[^"\\]*(?:\\.[^"\\]*)*")", regex_flags, error_code), Token::STRING);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^'[^'\\]*(?:\\.[^'\\]*)*')", regex_flags, error_code), Token::STRING);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^[0-9]+(\.[0-9]+)?\b)", regex_flags, error_code), Token::NUMBER);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^\n)", regex_flags, error_code), Token::NEWLINE);
        regex_tokens.emplace_back(MakeHolder<icu::RegexMatcher>(R"(^.)", regex_flags, error_code), Token::INVALID);
    }

private:
    Heap* heap;
    Handle<String> source;
    icu::UnicodeString source_view;
    size_t pos;
    size_t row;
    size_t col;
    UErrorCode error_code = U_ZERO_ERROR;
    std::vector<std::pair<Holder<icu::RegexMatcher>, Token>> regex_tokens;

    static constexpr uint32_t regex_flags = 0;
};

}