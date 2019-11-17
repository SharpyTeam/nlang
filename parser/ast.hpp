//
// Created by selya on 16.11.2019.
//

#ifndef NLANG_AST_HPP
#define NLANG_AST_HPP

#include <tokens.hpp>

#include <string>
#include <memory>

namespace nlang {

class Expression {
public:
    [[nodiscard]]
    virtual std::string ToString() const = 0;

    virtual ~Expression() = default;
};

class LiteralExpression : public Expression {
public:

};

class NumberLiteralExpression : public LiteralExpression {
public:
    const double number;

    explicit NumberLiteralExpression(double number)
        : number(number)
    {

    }

    [[nodiscard]]
    std::string ToString() const override {
        return std::to_string(number);
    }
};

class UnaryExpression : public Expression {
public:
    const Tokens::TokenType op;
    const std::shared_ptr<Expression> expression;

    UnaryExpression(Tokens::TokenType op, const std::shared_ptr<Expression> &expression)
        : op(op)
        , expression(expression)
    {

    }

    [[nodiscard]]
    std::string ToString() const override {
        return Tokens::token_to_string.at(op) + "(" + expression->ToString() + ")";
    }
};

class BinaryExpression : public Expression {
public:
    const Tokens::TokenType op;
    const std::shared_ptr<Expression> left;
    const std::shared_ptr<Expression> right;

    BinaryExpression(Tokens::TokenType op, const std::shared_ptr<Expression> &left, const std::shared_ptr<Expression> &right)
        : op(op)
        , left(left)
        , right(right)
    {

    }

    [[nodiscard]]
    std::string ToString() const override {
        return "(" + left->ToString() + " " + Tokens::token_to_string.at(op) + " " + right->ToString() + ")";
    }
};

}

#endif //NLANG_AST_HPP
