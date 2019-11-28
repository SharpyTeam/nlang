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

// TODO: Add human-readable token names as 3rd macro argument
#define TOKENS_LIST                     \
                                        \
TOKEN(COMMENT, "")                      \
TOKEN(OPERATOR_OR_PUNCTUATION, "")      \
TOKEN(IDENTIFIER, "")                   \
TOKEN(STRING, "")                       \
TOKEN(NUMBER, "")                       \
TOKEN(NEWLINE, "")                      \
TOKEN(SPACE, "")                        \
TOKEN(THE_EOF, "")                      \
                                        \
TOKEN(IF, "if")                         \
TOKEN(ELSE, "else")                     \
TOKEN(FOR, "for")                       \
TOKEN(IN, "in")                         \
TOKEN(DOWNTO, "downto")                 \
TOKEN(UNTIL, "until")                   \
TOKEN(STEP, "step")                     \
TOKEN(BREAK, "break")                   \
TOKEN(CONTINUE, "continue")             \
TOKEN(WHILE, "while")                   \
TOKEN(LOOP, "loop")                     \
TOKEN(FN, "fn")                         \
TOKEN(LET, "let")                       \
TOKEN(CONST, "const")                   \
                                        \
TOKEN(TO, "..")                         \
                                        \
TOKEN(LEFT_PAR, "(")                    \
TOKEN(RIGHT_PAR, ")")                   \
TOKEN(LEFT_BRACE, "{")                  \
TOKEN(RIGHT_BRACE, "}")                 \
TOKEN(COMMA, ",")                       \
TOKEN(SEMICOLON, ";")                   \
TOKEN(COLON, ":")                       \
                                        \
TOKEN(ASSIGN, "=")                      \
TOKEN(MUL, "*")                         \
TOKEN(DIV, "/")                         \
TOKEN(ADD, "+")                         \
TOKEN(SUB, "-")                         \
                                        \
TOKEN(THE_NULL, "null")                 \
TOKEN(TRUE, "true")                     \
TOKEN(FALSE, "false")                   \
                                        \
TOKEN(BIT_OR, "|")                      \
TOKEN(BIT_XOR, "^")                     \
TOKEN(BIT_AND, "&")                     \
TOKEN(TILDE, "~")                       \
TOKEN(LEFT_SHIFT, "<<")                 \
TOKEN(RIGHT_SHIFT, ">>")                \
TOKEN(AND, "and")                       \
TOKEN(OR, "or")                         \
TOKEN(XOR, "xor")                       \
TOKEN(NOT, "not")                       \
TOKEN(EQUALS, "==")                     \
TOKEN(NOT_EQUALS, "!=")                 \
TOKEN(GREATER, ">")                     \
TOKEN(GREATER_EQUALS, ">=")             \
TOKEN(LESS, "<")                        \
TOKEN(LESS_EQUALS, "<=")                \
                                        \
TOKEN(INVALID, "")                      \

class Tokens {
public:
    enum class TokenType {
#define TOKEN(token, value) token,
        TOKENS_LIST
#undef TOKEN
    };

    inline static const std::unordered_map<TokenType, std::string> token_names {
#define TOKEN(token, name) { TokenType::token, #token },
        TOKENS_LIST
#undef TOKEN
    };

    inline static const std::unordered_map<TokenType, std::string> token_to_string {
#define TOKEN(token, value) { TokenType::token, value },
        TOKENS_LIST
#undef TOKEN
    };

    inline static const std::unordered_set<TokenType> regex_tokens_to_lookup_in_tokens {
        TokenType::OPERATOR_OR_PUNCTUATION,
        TokenType::IDENTIFIER
    };

    inline static constexpr const std::regex_constants::syntax_option_type regex_flags =
        std::regex_constants::optimize | std::regex_constants::ECMAScript;

    // Comments should go first cause order matters (or scanner will scan two / signs instead of comment)
    inline static const std::vector<std::pair<std::regex, TokenType>> regex_tokens {
        { std::regex(R"(^\/\/.*?\n)", regex_flags),                 TokenType::COMMENT },
        { std::regex(R"(^\/\*.*?\*\/)", regex_flags),               TokenType::COMMENT },
        { std::regex(R"(^((==|!=|>=|<=|<<|>>)|\(|\)|\{|\}|;|,|=|:|\*|\/|\+|\-|!|>|<|\~|&|\||\^))",
                     regex_flags),                                        TokenType::OPERATOR_OR_PUNCTUATION },
        { std::regex(R"(^\b[a-zA-Z][a-zA-Z0-9_]*\b)", regex_flags), TokenType::IDENTIFIER },
        { std::regex(R"(^"[^"\\]*(?:\\.[^"\\]*)*")", regex_flags),  TokenType::STRING },
        { std::regex(R"(^\b[0-9]+(\.[0-9]+)?\b)", regex_flags),     TokenType::NUMBER },
        { std::regex(R"(^\n)", regex_flags),                        TokenType::NEWLINE },
        { std::regex(R"(^[\s\t\r]+)", regex_flags),                 TokenType::SPACE },
        { std::regex(R"(^$)", regex_flags),                         TokenType::THE_EOF }
    };

    inline static const std::unordered_map<std::string, TokenType> tokens {
#define TOKEN(token, name) { name, TokenType::token },
        TOKENS_LIST
#undef TOKEN
    };
};

}

#endif //NLANG_TOKENS_HPP
