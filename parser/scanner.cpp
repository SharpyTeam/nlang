//
// Created by selya on 05.11.2019.
//

#include "scanner.hpp"

namespace nlang {



std::vector<Scanner::Token> Scanner::ExtractTokens(const std::string_view &sv) {
    std::vector<Token> extracted;

    size_t offset = 0;

    while (true) {
        std::cmatch space_match;
        std::string buf(sv.data() + offset,
                        std::regex_search(sv.data() + offset, space_match, Tokens::any_space_character)
                        ? space_match.position() : sv.length() - offset);

        if (!buf.empty()) {
            if (auto it = Tokens::tokens.find(buf); it != Tokens::tokens.end()) {
                extracted.emplace_back(Token{it->second, it->first});
                offset += buf.size();
                continue;
            }
        }

        bool found = false;
        for (auto &[regex, token] : Tokens::regex_tokens) {
            std::cmatch match;
            if (std::regex_search(sv.data() + offset, match, regex)) {
                extracted.emplace_back(Token{token, match.str()});
                if (token == Tokens::TokenType::THE_EOF) return extracted;
                offset += match.position() + match.length();
                found = true;
                break;
            }
        }

        if (!found) {
            extracted.emplace_back(Token{Tokens::TokenType::INVALID, buf});
            offset += buf.size();
        }
    }
}

}
