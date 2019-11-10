//
// Created by selya on 09.11.2019.
//

#ifndef NLANG_TOKENS_HPP
#define NLANG_TOKENS_HPP

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>

namespace nlang {

class Tokens {
public:
    enum class TokenType {
        OPERATOR_OR_PUNCTUATION,
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

        THE_NULL,
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

    inline static const std::unordered_set<TokenType> regex_tokens_to_lookup_in_tokens = {
        TokenType::OPERATOR_OR_PUNCTUATION,
        TokenType::IDENTIFIER
    };

    inline static constexpr const std::regex_constants::syntax_option_type regex_flags =
        std::regex_constants::optimize | std::regex_constants::ECMAScript;

    inline static const std::vector<std::pair<std::regex, TokenType>> regex_tokens = {
        { std::regex(R"(^((==|!=|>=|<=)|\(|\)|\{|\}|,|=|\*|\/|\+|\-|!|>|<))",
                     regex_flags),                                              TokenType::OPERATOR_OR_PUNCTUATION },
        { std::regex(R"(^\b[a-zA-Z][a-zA-Z0-9_]*\b)", regex_flags), TokenType::IDENTIFIER },
        { std::regex(R"(^"[^"\\]*(?:\\.[^"\\]*)*")", regex_flags),  TokenType::STRING },
        { std::regex(R"(^\b[0-9]+(\.[0-9]+)?\b)", regex_flags),        TokenType::NUMBER },
        { std::regex(R"(^\n)", regex_flags),                        TokenType::NEWLINE },
        { std::regex(R"(^[\s\t\r]+)", regex_flags),                 TokenType::SPACE },
        { std::regex(R"(^$)", regex_flags),                         TokenType::THE_EOF }
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

        { "null",  TokenType::THE_NULL },
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

    inline static const std::vector<std::regex> regex_comments = {
            std::regex(R"(^\/\/.*?\n)", regex_flags),
            std::regex(R"(^\/\*.*?\*\/)", regex_flags),
    };
};

}

#endif //NLANG_TOKENS_HPP
