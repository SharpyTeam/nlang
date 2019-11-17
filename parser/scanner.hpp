//
// Created by selya on 05.11.2019.
//

#ifndef NLANG_SCANNER_HPP
#define NLANG_SCANNER_HPP

#include "tokens.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <stack>

namespace nlang {

class Scanner {
public:
    struct Token {
        Tokens::TokenType token;
        std::string value;
        int row;
        int column;
    };

private:
    size_t pos;
    std::vector<Token> tokens;
    std::stack<size_t> marks;

public:
    explicit Scanner(const std::string_view &source) noexcept;

    [[nodiscard]]
    const std::vector<Token> &GetTokens() const;

    void AddMark();
    void Restore();

    const Token &NextToken();
    size_t SkipTokens(Tokens::TokenType type);

private:
    static std::vector<Token> ExtractTokens(const std::string_view &sv);
};

}

#endif //NLANG_SCANNER_HPP
