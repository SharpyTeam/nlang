//
// Created by selya on 16.11.2019.
//

#ifndef NLANG_PARSER_HPP
#define NLANG_PARSER_HPP

#include "scanner.hpp"
#include <ast.hpp>

#include <functional>
#include <memory>
#include <utility>
#include <map>
#include <unordered_set>

namespace nlang {

class Parser {
public:
    std::shared_ptr<Expression> ParseExpression() {
        return ParseAssignExpression();
    }

    static std::shared_ptr<Parser> Create(const std::shared_ptr<Scanner> &scanner) {
        return std::shared_ptr<Parser>(new Parser(scanner));
    }

private:
    std::shared_ptr<Scanner> scanner;


    explicit Parser(const std::shared_ptr<Scanner> &scanner)
        : scanner(scanner)
    {
        scanner->ResetSkip();
        scanner->SetSkip(Token::SPACE);
        scanner->SetSkip(Token::COMMENT);
        scanner->SetSkip(Token::NEWLINE);
    }

#define BINARY(name, next, ...)                                                                 \
    std::shared_ptr<Expression> name() {                                                        \
        static std::unordered_set<Token> tokens { __VA_ARGS__ };                                       \
        auto expr = next();                                                                     \
        while (true) {                                                                          \
            auto mark = scanner->Mark();                                                        \
            if (auto token = scanner->NextToken(); tokens.find(token.token) != tokens.end()) {  \
                expr = std::make_shared<BinaryExpression>(token.token, expr, next());           \
            } else {                                                                            \
                mark.Apply();                                                                   \
                break;                                                                          \
            }                                                                                   \
        }                                                                                       \
        return expr;                                                                            \
    }

    std::shared_ptr<Expression> ParseParenthesizedExpression() {
        auto mark = scanner->Mark();
        scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        scanner->NextTokenAssert(Token::RIGHT_PAR);
        return expr;
    }

    std::shared_ptr<Expression> ParseBasicExpression() {
        auto mark = scanner->Mark();
        auto token = scanner->NextToken();
        switch (token.token) {
            case Token::LEFT_PAR:
                mark.Apply();
                return ParseParenthesizedExpression();

            case Token::IDENTIFIER:
                return std::make_shared<IdentifierExpression>(token.source);

            case Token::NUMBER:
                return std::make_shared<NumberLiteralExpression>(std::stod(token.source));

            case Token::STRING:
                return std::make_shared<StringLiteralExpression>(token.source);

            case Token::THE_NULL:
            case Token::TRUE:
            case Token::FALSE:
                return std::make_shared<LiteralExpression>(token.token);

            default:
                throw std::runtime_error("Kek");
        }
    }

    std::shared_ptr<Expression> ParsePostfixUnaryExpression() {
        static std::unordered_set<Token> tokens { Token::ADD_ADD, Token::SUB_SUB };
        auto expr = ParseBasicExpression();
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens.find(token.token) != tokens.end()) {
                expr = std::make_shared<PostfixExpression>(token.token, expr);
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    std::shared_ptr<Expression> ParsePrefixUnaryExpression() {
        static std::unordered_set<Token> tokens { Token::ADD, Token::SUB, Token::ADD_ADD, Token::SUB_SUB };
        std::stack<Token> operators;
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens.find(token.token) != tokens.end()) {
                operators.push(token.token);
            } else {
                mark.Apply();
                auto expr = ParsePostfixUnaryExpression();
                while (!operators.empty()) {
                    expr = std::make_shared<UnaryExpression>(operators.top(), expr);
                    operators.pop();
                }
                return expr;
            }
        }
    }

    BINARY(ParseMultiplicativeExpression, ParsePrefixUnaryExpression, Token::MUL, Token::DIV, Token::REMAINDER)
    BINARY(ParseAdditiveExpression, ParseMultiplicativeExpression, Token::ADD, Token::SUB)

    std::shared_ptr<Expression> ParseRangeExpression() {
        return ParseAdditiveExpression();
    }

    BINARY(ParseFunctionAsOperatorExpression, ParseRangeExpression, Token::IDENTIFIER)
    BINARY(ParseComparisonExpression, ParseFunctionAsOperatorExpression, Token::LESS_EQUALS, Token::GREATER_EQUALS, Token::LESS, Token::GREATER)
    BINARY(ParseEqualityExpression, ParseComparisonExpression, Token::EQUALS, Token::NOT_EQUALS)
    BINARY(ParseConjunctionExpression, ParseEqualityExpression, Token::AND)
    BINARY(ParseDisjunctionExpression, ParseConjunctionExpression, Token::OR)
    BINARY(ParseAssignExpression, ParseDisjunctionExpression, Token::ASSIGN, Token::ASSIGN_ADD, Token::ASSIGN_SUB, Token::ASSIGN_MUL, Token::ASSIGN_DIV, Token::ASSIGN_REMAINDER)

    /*std::shared_ptr<Statement> ParseFunctionDeclarationStatement() {
        std::string name;
        std::vector<std::string> args_list;
        std::vector<std::shared_ptr<Statement>> body;
        scanner->NextTokenAssert(Token::FN);
        auto mark = scanner->Mark();
        auto tok = scanner->NextToken();
        if (tok.token == Token::IDENTIFIER) {
            name = tok.source;
        } else {
            mark.Apply();
        }
        scanner->NextTokenAssert(Token::LEFT_PAR);
        while (true) {
            mark = scanner->Mark();
            if (scanner->NextToken().token == Token::RIGHT_PAR) {
                break;
            }
            mark.Apply();
            args_list.emplace_back(scanner->NextTokenAssert(Token::IDENTIFIER).source);
            scanner->NextTokenAssert(Token::COMMA);
        }
        scanner->NextTokenAssert(Token::LEFT_BRACE);
        while (true) {
            mark = scanner->Mark();
            if (scanner->NextToken().token == Token::RIGHT_BRACE) {
                break;
            }
            mark.Apply();
            body.emplace_back(ParseSimpleStatement());
        }
        return std::make_shared<FunctionDeclarationStatement>(name, args_list, body);
    }*/
};

}

#endif //NLANG_PARSER_HPP
