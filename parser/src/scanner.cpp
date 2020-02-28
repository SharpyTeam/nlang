#include <parser/scanner.hpp>

#include <stdexcept>

namespace nlang {

Scanner::ScannerImpl::ScannerImpl(std::shared_ptr<ICharStream> char_stream)
    : row(1)
    , column(1)
    , char_stream_cache_offset(0)
    , char_stream(std::move(char_stream))
    , current_iterator(this)
    , end_iterator(this, -1)
{

}

TokenInstance Scanner::ScannerImpl::NextToken() {
    static constexpr const std::regex_constants::syntax_option_type regex_flags =
        std::regex_constants::optimize | std::regex_constants::ECMAScript;

    static const std::vector<std::pair<std::regex, Token>> regex_tokens {
        { std::regex(R"(^[ \t\r]+)", regex_flags),                 Token::SPACE },
        { std::regex(R"(^\/\/.*?[\n$])", regex_flags),                 Token::COMMENT },
        { std::regex(R"(^\/\*.*?\*\/)", regex_flags),               Token::COMMENT },
        { std::regex(R"(^((\+\+|\-\-|\+=|-=|\*=|\/=|\%=|==|!=|>=|<=|<<|>>)|\(|\)|\{|\}|;|,|=|\*|\/|\+|\-|!|>|<|\~|&|\||\^))",
                     regex_flags),                                        Token::OPERATOR_OR_PUNCTUATION },
        { std::regex(R"(^([^\x00-\x7F]|[a-zA-Z])([^\x00-\x7F]|[a-zA-Z0-9_])*)", regex_flags), Token::IDENTIFIER },
        { std::regex(R"(^"[^"\\]*(?:\\.[^"\\]*)*")", regex_flags),  Token::STRING },
        { std::regex(R"(^[0-9]+(\.[0-9]+)?\b)", regex_flags),       Token::NUMBER },
        { std::regex(R"(^\n)", regex_flags),                        Token::NEWLINE }
    };

    if (char_stream_cache.size() >= 4096) {
        CutCharCache(current_iterator.pos);
    }

    size_t invalid_pos = 0;
    std::string invalid_buf;

    while (current_iterator != end_iterator) {
        for (auto &[regex, token] : regex_tokens) {
            std::match_results<CachingCharStreamIterator> match;
            if (std::regex_search(current_iterator, end_iterator, match, regex, std::regex_constants::match_continuous)) {
                if (!invalid_buf.empty()) {
                    std::string ib;
                    std::swap(ib, invalid_buf);
                    return TokenInstance { Token::INVALID, invalid_pos, row, column - invalid_buf.length(), ib };
                }

                auto str = match.str();

                Token actual_token = token;
                if (token == Token::OPERATOR_OR_PUNCTUATION || token == Token::IDENTIFIER) {
                    try {
                        actual_token = TokenUtils::GetTokenByText(str);
                    } catch (...) {}
                }

                const size_t saved_row = row;
                const size_t saved_column = column;

                for (char i : str) {
                    ++column;
                    if (i == '\n') {
                        row++;
                        column = 1;
                    }
                }

                size_t pos_in_string = current_iterator.pos;
                std::advance(current_iterator, str.length());
                if (actual_token == Token::STRING) {
                    str = str.substr(1, str.size() - 2);
                }
                return TokenInstance { actual_token, pos_in_string, saved_row, saved_column, str };
            }
        }

        if (invalid_buf.empty()) {
            invalid_pos = current_iterator.pos;
        }
        invalid_buf += *current_iterator;
        ++column;
        ++current_iterator;
    }

    if (!invalid_buf.empty()) {
        std::string ib;
        std::swap(ib, invalid_buf);
        return TokenInstance { Token::INVALID, invalid_pos, row, column - invalid_buf.length(), ib };
    }

    return TokenInstance { Token::THE_EOF, current_iterator.pos, row, column, "" };
}

bool Scanner::TrySkipToken(Token token, Scanner::AdvanceBehaviour advance_behaviour) {
    auto mark = Mark();
    auto &t = NextToken(advance_behaviour);
    if (t.token == token) {
        return true;
    }
    mark.Apply();
    return false;
}

const TokenInstance &Scanner::NextTokenAssert(Token token, AdvanceBehaviour advance_behaviour) {
    auto mark = Mark();
    auto &ts = NextToken(advance_behaviour);
    if (ts.token != token) {
        mark.Apply();
        throw std::runtime_error("Expected " + TokenUtils::GetTokenName(token) + ", found " +
                                 TokenUtils::GetTokenName(ts.token));
    }
    return ts;
}

const TokenInstance &Scanner::NextTokenLookahead(Scanner::AdvanceBehaviour advance_behaviour) {
    auto mark = Mark();
    auto &ts = NextToken(advance_behaviour);
    mark.Apply();
    return ts;
}

const TokenInstance &Scanner::NextToken(AdvanceBehaviour advance_behaviour) {
    if (advance_behaviour == AdvanceBehaviour::NO_IGNORE) {
        return NextTokenWithoutIgnore();
    }
    const TokenInstance *token;
    do {
        token = &NextTokenWithoutIgnore();
    } while (tokens_to_ignore.find(token->token) != tokens_to_ignore.end());
    return *token;
}

const TokenInstance &Scanner::NextTokenWithoutIgnore() {
    if (pos == tokens.size()) {
        if (!tokens.empty() && tokens[pos - 1].token == Token::THE_EOF) {
            throw std::runtime_error("No tokens left");
        }
        tokens.emplace_back(impl.NextToken());
    }
    return tokens[pos++];
}

void Scanner::ResetIgnore() {
    tokens_to_ignore.clear();
}

bool Scanner::IsIgnoring(Token token) const {
    return tokens_to_ignore.find(token) != tokens_to_ignore.end();
}

void Scanner::SetIgnore(Token token, bool ignore) {
    if (ignore) {
        tokens_to_ignore.emplace(token);
    } else {
        tokens_to_ignore.erase(token);
    }
}

std::shared_ptr<Scanner> Scanner::Create(const std::shared_ptr<ICharStream> &char_stream) {
    return std::shared_ptr<Scanner>(new Scanner(char_stream));
}

Scanner::Scanner(std::shared_ptr<ICharStream> char_stream)
    : impl(std::move(char_stream))
    , pos(0)
{

}

Scanner::BookMark Scanner::Mark() const {
    return BookMark(const_cast<Scanner *>(this));
}

}
