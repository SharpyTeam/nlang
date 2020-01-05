//
// Created by selya on 09.11.2019.
//

#ifndef NLANG_TOKEN_HPP
#define NLANG_TOKEN_HPP

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <stdexcept>

namespace nlang {

// TODO: Add human-readable token names as 3rd macro argument
#define TOKENS_LIST                 \
                                    \
T(COMMENT, "")                      \
T(OPERATOR_OR_PUNCTUATION, "")      \
T(IDENTIFIER, "")                   \
T(STRING, "")                       \
T(NUMBER, "")                       \
T(NEWLINE, "")                      \
T(SPACE, "")                        \
T(THE_EOF, "")                      \
                                    \
T(IF, "if")                         \
T(ELSE, "else")                     \
T(FOR, "for")                       \
T(WHILE, "while")                   \
T(LOOP, "loop")                     \
T(FN, "fn")                         \
T(LET, "let")                       \
T(CONST, "const")                   \
T(RETURN, "return")                 \
T(CONTINUE, "continue")             \
T(BREAK, "break")                   \
                                    \
T(LEFT_PAR, "(")                    \
T(RIGHT_PAR, ")")                   \
T(LEFT_BRACE, "{")                  \
T(RIGHT_BRACE, "}")                 \
T(COMMA, ",")                       \
T(SEMICOLON, ";")                   \
                                    \
T(ASSIGN, "=")                      \
T(ASSIGN_ADD, "+=")                 \
T(ASSIGN_SUB, "-=")                 \
T(ASSIGN_MUL, "*=")                 \
T(ASSIGN_DIV, "/=")                 \
T(ASSIGN_REMAINDER, "%=")           \
T(MUL, "*")                         \
T(DIV, "/")                         \
T(ADD, "+")                         \
T(SUB, "-")                         \
T(REMAINDER, "%")                   \
T(ADD_ADD, "++")                    \
T(SUB_SUB, "--")                    \
                                    \
T(THE_NULL, "null")                 \
T(TRUE, "true")                     \
T(FALSE, "false")                   \
                                    \
T(BIT_OR, "|")                      \
T(BIT_XOR, "^")                     \
T(BIT_AND, "&")                     \
T(TILDE, "~")                       \
T(LEFT_SHIFT, "<<")                 \
T(RIGHT_SHIFT, ">>")                \
T(AND, "and")                       \
T(OR, "or")                         \
T(XOR, "xor")                       \
T(NOT, "not")                       \
T(EQUALS, "==")                     \
T(NOT_EQUALS, "!=")                 \
T(GREATER, ">")                     \
T(GREATER_EQUALS, ">=")             \
T(LESS, "<")                        \
T(LESS_EQUALS, "<=")                \
                                    \
T(INVALID, "")                      \

enum class Token : uint8_t {
#define T(token, value) token,
    TOKENS_LIST
#undef T
};

struct TokenInstance {
    Token token;
    size_t pos;
    size_t row;
    size_t column;
    std::string source;
};

class TokenUtils {
public:
    static const std::string &TokenToString(Token token) {
        return token_to_string.at(token);
    }

    static const std::string &TokenToSource(Token token) {
        return token_to_source.at(token);
    }

    static Token SourceToToken(const std::string &source) {
        if (source.empty()) {
            throw std::runtime_error("Can't convert empty source to token");
        }
        return source_to_token.at(source);
    }

private:
    inline static const std::unordered_map<Token, std::string> token_to_string {
#define T(token, value) { Token::token, #token },
        TOKENS_LIST
#undef T
    };

    inline static const std::unordered_map<Token, std::string> token_to_source {
#define T(token, value) { Token::token, value },
        TOKENS_LIST
#undef T
    };

    inline static const std::unordered_map<std::string, Token> source_to_token {
#define T(token, value) { value, Token::token },
        TOKENS_LIST
#undef T
    };
};

}

#endif //NLANG_TOKEN_HPP
