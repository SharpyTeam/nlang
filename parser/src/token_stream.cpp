#include <parser/token_stream.hpp>


namespace nlang {


TokenStream::TokenStream(UString&& source_)
    : source(std::move(source_))
    , source_view(source.SubStringView())
    , pos(0)
    , row(1)
    , col(1)
{
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^[ \t\r]+)", regex_flags, error_code), Token::SPACE);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^\/\/.*?(\n|$))", regex_flags, error_code), Token::COMMENT);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^\/\*.*?\*\/)", regex_flags, error_code), Token::COMMENT);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(
        R"(^((\+\+|\-\-|\+=|-=|\*=|\/=|\%=|==|!=|>=|<=|<<|>>)|\(|\)|\{|\}|\[|\]|;|:|,|\.|=|\*|\/|\+|\-|!|>|<|\~|&|\||\^))",
        regex_flags, error_code), Token::OPERATOR_OR_PUNCTUATION);
    regex_tokens.emplace_back(
        MakeUnique<icu::RegexMatcher>(R"(^([^\x00-\x7F]|[a-zA-Z_])([^\x00-\x7F]|[a-zA-Z0-9_])*)", regex_flags,
                                      error_code), Token::IDENTIFIER);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^"[^"\\]*(?:\\.[^"\\]*)*")", regex_flags, error_code), Token::STRING);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^'[^'\\]*(?:\\.[^'\\]*)*')", regex_flags, error_code), Token::STRING);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^[0-9]+(\.[0-9]+)?\b)", regex_flags, error_code), Token::NUMBER);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^\n)", regex_flags, error_code), Token::NEWLINE);
    regex_tokens.emplace_back(MakeUnique<icu::RegexMatcher>(R"(^.)", regex_flags, error_code), Token::INVALID);
}

TokenStream::TokenStream(const UString &source_)
    : TokenStream(UString(source_))
{}

bool TokenStream::HasNext() {
    return pos != -1;
}

TokenInstance TokenStream::Next() {
    NLANG_ASSERT(HasNext());

    while (pos != source.GetLength()) {
        for (auto& [regex, token] : regex_tokens) {
            regex->reset(source_view);
            if (regex->find(error_code)) {
                UString str(regex->group(error_code));

                Token actual_token = token;
                if (token == Token::OPERATOR_OR_PUNCTUATION || token == Token::IDENTIFIER) {
                    try {
                        actual_token = TokenUtils::GetTokenByText(str);
                    } catch (std::out_of_range&) {}
                }

                const int32_t saved_row = row;
                const int32_t saved_column = col;

                for (int32_t i = 0; i < str.GetLength(); ++i) {
                    ++col;
                    if (str.GetCharAt(i) == '\n') {
                        row++;
                        col = 1;
                    }
                }

                const int32_t pos_in_string = pos;
                pos += str.GetLength();
                source_view = source.SubStringView(pos);
                return TokenInstance { actual_token, pos_in_string, str.GetLength(), saved_row, saved_column, std::move(str) };
            }
        }
    }

    pos = -1;
    return TokenInstance { Token::THE_EOF, pos, 0, row, col, UString() };
}

}