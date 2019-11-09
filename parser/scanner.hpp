//
// Created by selya on 05.11.2019.
//

#ifndef NLANG_SCANNER_HPP
#define NLANG_SCANNER_HPP

#include "tokens.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace nlang {

class Scanner {
public:
    struct Token {
        Tokens::TokenType token;
        std::string value;
    };

private:
    std::vector<Token> tokens;

public:
    explicit Scanner(const std::string_view &source) noexcept
        : tokens(ExtractTokens(source)) {}

private:
    static std::vector<Token> ExtractTokens(const std::string_view &sv);
};

}

#endif //NLANG_SCANNER_HPP
