//
// Created by selya on 16.11.2019.
//

#ifndef NLANG_PARSER_HPP
#define NLANG_PARSER_HPP

#include "scanner.hpp"
#include <ast.hpp>

#include <memory>

namespace nlang {

class Parser {
    // TODO: Move definitions to .cpp?
    // TODO: Implement all binary operators
public:
    explicit Parser(const std::shared_ptr<Scanner> &scanner)
        : scanner(scanner)
    {

    }

    std::shared_ptr<Expression> ParseExpression() {
        return ParseAddExpression();
    }

private:
    std::shared_ptr<Expression> ParseParExpression() {
        scanner->AddMark();
        scanner->SkipTokens(Tokens::TokenType::SPACE);
        scanner->NextToken();
        auto expr = ParseExpression();
        scanner->SkipTokens(Tokens::TokenType::SPACE);
        scanner->NextToken();
        return expr;
    }

    std::shared_ptr<Expression> ParseAtomExpression() {
        scanner->AddMark();
        scanner->SkipTokens(Tokens::TokenType::SPACE);
        auto token = scanner->NextToken();
        switch (token.token) {
            case Tokens::TokenType::NUMBER:
                return std::make_shared<NumberLiteralExpression>(std::stod(token.value));
            case Tokens::TokenType::LEFT_PAR:
                scanner->Restore();
                return ParseParExpression();
        }

        // TODO: Replace with more generalized parser error reporting function
        throw std::runtime_error("Unexpected " + std::string(Tokens::token_names.at(token.token)) + " at line " + std::to_string(token.row) + ", " + std::to_string(token.column));
    }

    std::shared_ptr<Expression> ParseUnaryExpression() {
        scanner->AddMark();
        scanner->SkipTokens(Tokens::TokenType::SPACE);
        if (auto token = scanner->NextToken();
            token.token == Tokens::TokenType::ADD
            || token.token == Tokens::TokenType::SUB) {
            return std::make_shared<UnaryExpression>(token.token, ParseAtomExpression());
        }
        scanner->Restore();
        return ParseAtomExpression();
    }

    std::shared_ptr<Expression> ParseMulExpression() {
        auto expr = ParseUnaryExpression();
        while (true) {
            scanner->AddMark();
            scanner->SkipTokens(Tokens::TokenType::SPACE);
            if (auto token = scanner->NextToken();
                token.token == Tokens::TokenType::MUL
                || token.token == Tokens::TokenType::DIV) {
                expr = std::make_shared<BinaryExpression>(token.token, expr, ParseUnaryExpression());
            } else {
                scanner->Restore();
                break;
            }
        }
        return expr;
    }

    std::shared_ptr<Expression> ParseAddExpression() {
        auto expr = ParseMulExpression();
        while (true) {
            scanner->AddMark();
            scanner->SkipTokens(Tokens::TokenType::SPACE);
            if (auto token = scanner->NextToken();
                token.token == Tokens::TokenType::ADD
                || token.token == Tokens::TokenType::SUB) {
                expr = std::make_shared<BinaryExpression>(token.token, expr, ParseMulExpression());
            } else {
                scanner->Restore();
                break;
            }
        }
        return expr;
    }

    std::shared_ptr<Scanner> scanner;
};

}

#endif //NLANG_PARSER_HPP
