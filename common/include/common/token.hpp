#pragma once

#include <utils/macro.hpp>
#include <utils/strings.hpp>

#include <unordered_map>

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
T(CLASS, "class")                   \
T(FN, "fn")                         \
T(OP, "op")                         \
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
T(LEFT_BRACKET, "[")                \
T(RIGHT_BRACKET, "]")               \
T(DOT, ".")                         \
T(COMMA, ",")                       \
T(COLON, ":")                       \
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
T(THE_TRUE, "true")                 \
T(THE_FALSE, "false")               \
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
    int32_t pos;
    int32_t length;
    int32_t row;
    int32_t column;
    UString text;
};

class TokenUtils {
public:
    static const UString& GetTokenName(Token token) {
        return token_to_name.at(token);
    }

    static const UString& GetTokenText(Token token) {
        const UString& text = token_to_text.at(token);
        NLANG_ASSERT(text);
        return text;
    }

    static Token GetTokenByText(const UString& text) {
        NLANG_ASSERT(text);
        return text_to_token.at(text);
    }

private:
    inline static const std::unordered_map<Token, UString> token_to_name {
#define T(token, value) { Token::token, #token },
        TOKENS_LIST
#undef T
    };

    inline static const std::unordered_map<Token, UString> token_to_text {
#define T(token, value) { Token::token, value },
        TOKENS_LIST
#undef T
    };

    inline static const std::unordered_map<UString, Token> text_to_token {
#define T(token, value) { value, Token::token },
        TOKENS_LIST
#undef T
    };
};

}
