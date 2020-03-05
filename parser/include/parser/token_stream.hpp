#pragma once

#include "char_stream.hpp"
#include "stream_cache.hpp"

#include <utils/defs.hpp>
#include <common/token.hpp>

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
        return !reached_end;
    }

    TokenInstance Next() {
        NLANG_ASSERT(!reached_end);

        static constexpr const std::regex_constants::syntax_option_type regex_flags =
            std::regex_constants::optimize | std::regex_constants::ECMAScript;

        static const std::vector<std::pair<std::regex, Token>> regex_tokens {
            { std::regex(R"(^[ \t\r]+)", regex_flags),                 Token::SPACE },
            { std::regex(R"(^\/\/.*?(\n|$))", regex_flags),                 Token::COMMENT },
            { std::regex(R"(^\/\*.*?\*\/)", regex_flags),               Token::COMMENT },
            { std::regex(R"(^((\+\+|\-\-|\+=|-=|\*=|\/=|\%=|==|!=|>=|<=|<<|>>)|\(|\)|\{|\}|;|,|=|\*|\/|\+|\-|!|>|<|\~|&|\||\^))",
                         regex_flags),                                        Token::OPERATOR_OR_PUNCTUATION },
            { std::regex(R"(^([^\x00-\x7F]|[a-zA-Z_])([^\x00-\x7F]|[a-zA-Z0-9_])*)", regex_flags), Token::IDENTIFIER },
            { std::regex(R"(^"[^"\\]*(?:\\.[^"\\]*)*")", regex_flags),  Token::STRING },
            { std::regex(R"(^[0-9]+(\.[0-9]+)?\b)", regex_flags),       Token::NUMBER },
            { std::regex(R"(^\n)", regex_flags),                        Token::NEWLINE }
        };

        size_t invalid_pos = 0;
        std::string invalid_buf;

        auto it = cache.begin();

        while (it != cache.end()) {
            for (auto &[regex, token] : regex_tokens) {
                std::match_results<StreamCache<ICharStream>::StreamCacheIterator> match;
                if (std::regex_search(it, cache.end(), match, regex, std::regex_constants::match_continuous)) {
                    if (!invalid_buf.empty()) {
                        std::string ib;
                        std::swap(ib, invalid_buf);
                        it.CutCacheToThis();
                        return TokenInstance { Token::INVALID, invalid_pos, row, col - invalid_buf.length(), ib };
                    }

                    auto str = match.str();

                    Token actual_token = token;
                    if (token == Token::OPERATOR_OR_PUNCTUATION || token == Token::IDENTIFIER) {
                        try {
                            actual_token = TokenUtils::GetTokenByText(str);
                        } catch (std::out_of_range&) {}
                    }

                    const size_t saved_row = row;
                    const size_t saved_column = col;

                    for (char i : str) {
                        ++col;
                        if (i == '\n') {
                            row++;
                            col = 1;
                        }
                    }

                    size_t pos_in_string = it.GetPosition();
                    std::advance(it, str.length());
                    if (actual_token == Token::STRING) {
                        str = str.substr(1, str.size() - 2);
                    }
                    it.CutCacheToThis();
                    return TokenInstance { actual_token, pos_in_string, saved_row, saved_column, str };
                }
            }

            if (invalid_buf.empty()) {
                invalid_pos = it.GetPosition();
            }
            invalid_buf += *it;
            ++col;
            ++it;
        }

        if (!invalid_buf.empty()) {
            std::string ib;
            std::swap(ib, invalid_buf);
            it.CutCacheToThis();
            return TokenInstance { Token::INVALID, invalid_pos, row, col - invalid_buf.length(), ib };
        }

        it.CutCacheToThis();
        reached_end = true;
        return TokenInstance { Token::THE_EOF, it.GetPosition(), row, col, "" };
    }

    static std::unique_ptr<TokenStream> New(std::unique_ptr<ICharStream>&& char_stream) {
        return std::unique_ptr<TokenStream>(new TokenStream(std::move(char_stream)));
    }

private:
    TokenStream(std::unique_ptr<ICharStream>&& char_stream)
        : cache(std::move(char_stream))
        , row(1)
        , col(1)
        , reached_end(false)
    {

    }

private:
    StreamCache<ICharStream> cache;
    size_t row;
    size_t col;
    bool reached_end;
};

}