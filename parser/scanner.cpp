//
// Created by selya on 05.11.2019.
//

#include "scanner.hpp"

namespace nlang {

std::vector<Scanner::Token> Scanner::ExtractTokens(const std::string_view &sv) {
    std::vector<Token> extracted;
    size_t offset = 0;
    std::string invalid_buf;

    while (true) {
        bool found = false;
        for (auto &[regex, token] : Tokens::regex_tokens) {
            std::cmatch match;
            if (std::regex_search(sv.data() + offset, match, regex, std::regex_constants::match_continuous)) {
                extracted.emplace_back(Token { token, match.str() });
                if (Tokens::regex_tokens_to_lookup_in_tokens.find(token) != Tokens::regex_tokens_to_lookup_in_tokens.end()) {
                    if (auto it = Tokens::tokens.find(match.str()); it != Tokens::tokens.end()) {
                        extracted[extracted.size() - 1].token = it->second;
                    }
                }
                if (token == Tokens::TokenType::THE_EOF) return extracted;
                offset += match.position() + match.length();
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
