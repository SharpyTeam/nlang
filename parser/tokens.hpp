//
// Created by selya on 09.11.2019.
//

#ifndef NLANG_TOKENS_HPP
#define NLANG_TOKENS_HPP

#include <unordered_map>
#include <string>
#include <regex>

namespace nlang {

class Tokens {
public:
    enum class TokenType {
        IDENTIFIER,
        STRING,
        NUMBER,
        SPACE,
        NEWLINE,
        THE_EOF,

        IF,
        ELSE,
        FOR,
        WHILE,
        LOOP,
        FN,
        LET,

        LEFT_PAR,
        RIGHT_PAR,
        LEFT_BRACE,
        RIGHT_BRACE,
        COMMA,

        ASSIGN,
        MUL,
        DIV,
        ADD,
        SUB,

        TRUE,
        FALSE,
        AND,
        OR,
        XOR,
        NOT,
        EQUALS,
        NOT_EQUALS,
        GREATER,
        GREATER_EQUALS,
        LESS,
        LESS_EQUALS,

        INVALID
    };

    inline static const std::regex any_space_character = std::regex(R"([\n\s\t\r])");

    inline static const std::vector<std::pair<std::regex, TokenType>> regex_tokens = {
        { std::regex(R"(^\b[a-zA-Z][a-zA-Z0-9_]+\b)"), TokenType::IDENTIFIER },
        { std::regex(R"(^"[^"]+")"),                   TokenType::STRING },
        { std::regex(R"(^\b[0-9]+\b)"),                TokenType::NUMBER },
        { std::regex(R"(^\n)"),                        TokenType::NEWLINE },
        { std::regex(R"(^[\s\t\r]+)"),                 TokenType::SPACE },
        { std::regex(R"(^$)"),                         TokenType::THE_EOF }
    };

    inline static const std::unordered_map<std::string, TokenType> tokens = {
        { "if",    TokenType::IF },
        { "else",  TokenType::ELSE },
        { "for",   TokenType::FOR },
        { "while", TokenType::WHILE },
        { "loop",  TokenType::LOOP },
        { "fn",    TokenType::FN },
        { "let",   TokenType::LET },

        { "(",     TokenType::LEFT_PAR },
        { ")",     TokenType::RIGHT_PAR },
        { "{",     TokenType::LEFT_BRACE },
        { "}",     TokenType::RIGHT_BRACE },
        { ",",     TokenType::COMMA },


        { "=",     TokenType::ASSIGN },
        { "*",     TokenType::MUL },
        { "/",     TokenType::DIV },
        { "+",     TokenType::ADD },
        { "-",     TokenType::SUB },

        { "true",  TokenType::TRUE },
        { "false", TokenType::FALSE },
        { "and",   TokenType::AND },
        { "or",    TokenType::OR },
        { "xor",   TokenType::XOR },
        { "!",     TokenType::NOT },
        { "==",    TokenType::EQUALS },
        { "!=",    TokenType::NOT_EQUALS },
        { ">",     TokenType::GREATER },
        { ">=",    TokenType::GREATER_EQUALS },
        { "<",     TokenType::LESS },
        { "<=",    TokenType::LESS_EQUALS },
    };
};

}

#endif //NLANG_TOKENS_HPP
