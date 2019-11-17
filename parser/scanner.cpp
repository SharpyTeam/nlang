//
// Created by selya on 05.11.2019.
//

#include "scanner.hpp"

#include <stdexcept>

namespace nlang {

Scanner::Scanner(const std::string_view &source) noexcept
    : pos(0), tokens(ExtractTokens(source)) {}

const std::vector<Scanner::Token> &Scanner::GetTokens() const {
    return tokens;
}

void Scanner::AddMark() {
    marks.push(pos);
}

void Scanner::Restore() {
    if (marks.empty()) {
        throw std::runtime_error("No marks left");
    }
    pos = marks.top();
    marks.pop();
}

const Scanner::Token &Scanner::NextToken() {
    if (pos == tokens.size()) {
        throw std::runtime_error("No tokens left");
    }
    return tokens[pos++];
}

size_t Scanner::SkipTokens(Tokens::TokenType type) {
    size_t count = 0;
    while (pos < tokens.size() && tokens[pos].token == type) {
        count++;
        pos++;
    }
    return count;
}

std::vector<Scanner::Token> Scanner::ExtractTokens(const std::string_view &sv) {
    std::vector<Token> extracted;
    size_t offset = 0;
    std::string invalid_buf;

    size_t row = 1;
    size_t column = 1;

    while (true) {
        bool found = false;

        for (auto &[regex, token] : Tokens::regex_tokens) {
            std::cmatch match;
            if (std::regex_search(sv.data() + offset, match, regex, std::regex_constants::match_continuous)) {
                auto str = match.str();
                extracted.emplace_back(Token { token, str, int(row), int(column) });
                if (Tokens::regex_tokens_to_lookup_in_tokens.find(token) != Tokens::regex_tokens_to_lookup_in_tokens.end()) {
                    if (auto it = Tokens::tokens.find(str); it != Tokens::tokens.end()) {
                        extracted[extracted.size() - 1].token = it->second;
                    }
                }
                if (token == Tokens::TokenType::THE_EOF) return extracted;
                for (char i : str) {
                    ++column;
                    if (i == '\n') {
                        row++;
                        column = 1;
                    }
                }
                offset += str.length();
                found = true;
                break;
            }
        }

        if (!found) {
            invalid_buf += sv[offset++];
        } else if (!invalid_buf.empty()) {
            extracted.emplace(std::prev(extracted.end()), Token { Tokens::TokenType::INVALID, invalid_buf });
            invalid_buf.clear();
        }
    }
}

}
