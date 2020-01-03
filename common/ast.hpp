//
// Created by selya on 16.11.2019.
//

#ifndef NLANG_AST_HPP
#define NLANG_AST_HPP

#include "token.hpp"

#include <string>
#include <memory>
#include <utility>

namespace nlang {

class ASTNode;
class LiteralExpression;
class NumberLiteralExpression;
class StringLiteralExpression;
class IdentifierExpression;
class BinaryExpression;
class UnaryExpression;
class PostfixExpression;
class ExpressionStatement;
class ReturnStatement;
class BlockStatement;
class VarDefStatement;
class FunctionCallExpression;
class FunctionDefExpression;


class ASTVisitor {
public:
    virtual void Visit(ASTNode&) = 0;
    virtual void Visit(LiteralExpression&) = 0;
    virtual void Visit(NumberLiteralExpression&) = 0;
    virtual void Visit(StringLiteralExpression&) = 0;
    virtual void Visit(IdentifierExpression&) = 0;
    virtual void Visit(BinaryExpression&) = 0;
    virtual void Visit(UnaryExpression&) = 0;
    virtual void Visit(PostfixExpression&) = 0;
    virtual void Visit(ExpressionStatement&) = 0;
    virtual void Visit(ReturnStatement&) = 0;
    virtual void Visit(BlockStatement&) = 0;
    virtual void Visit(VarDefStatement&) = 0;
    virtual void Visit(FunctionCallExpression&) = 0;
    virtual void Visit(FunctionDefExpression&) = 0;
};

#define VISITOR_ACCEPT                          \
virtual void Accept(ASTVisitor& visitor) {      \
    visitor.Visit(*this);                       \
}

class ASTNode {
public:
    VISITOR_ACCEPT
    virtual ~ASTNode() = default;
};

class Expression : public ASTNode {
public:
    VISITOR_ACCEPT
};

class LiteralExpression : public Expression {
public:
    VISITOR_ACCEPT

    const Token type;

    explicit LiteralExpression(Token type)
        : type(type)
    {

    }
};

class NumberLiteralExpression : public LiteralExpression {
public:
    VISITOR_ACCEPT

    const double number;

    explicit NumberLiteralExpression(double number)
        : LiteralExpression(Token::NUMBER)
        , number(number)
    {

    }
};

class StringLiteralExpression : public LiteralExpression {
public:
    VISITOR_ACCEPT

    const std::string string;

    explicit StringLiteralExpression(std::string string)
        : LiteralExpression(Token::STRING)
        , string(std::move(string))
    {

    }
};

class IdentifierExpression : public Expression {
public:
    VISITOR_ACCEPT

    const std::string identifier;

    explicit IdentifierExpression(std::string identifier)
        : identifier(std::move(identifier))
    {

    }
};

class BinaryExpression : public Expression {
public:
    VISITOR_ACCEPT

    const Token op;
    const std::shared_ptr<Expression> left;
    const std::shared_ptr<Expression> right;

    BinaryExpression(Token op, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
        : op(op)
        , left(std::move(left))
        , right(std::move(right))
    {

    }
};

class UnaryExpression : public Expression {
public:
    VISITOR_ACCEPT

    const Token op;
    const std::shared_ptr<Expression> expression;

    UnaryExpression(Token op, std::shared_ptr<Expression> expression)
        : op(op)
        , expression(std::move(expression))
    {

    }
};

class PostfixExpression : public Expression {
public:
    VISITOR_ACCEPT

    const Token op;
    const std::shared_ptr<Expression> expression;

    PostfixExpression(Token op, std::shared_ptr<Expression> expression)
        : op(op)
        , expression(std::move(expression))
    {

    }
};

class Statement : public ASTNode {
public:
    VISITOR_ACCEPT
};

class ExpressionStatement : public Statement {
public:
    VISITOR_ACCEPT

    const std::shared_ptr<Expression> expression;

    explicit ExpressionStatement(std::shared_ptr<Expression> expression)
        : expression(std::move(expression))
    {

    }
};

class ReturnStatement : public Statement {
public:
    VISITOR_ACCEPT

    const std::shared_ptr<Expression> return_expression;

    explicit ReturnStatement(std::shared_ptr<Expression> return_expression = nullptr)
        : return_expression(std::move(return_expression))
    {

    }
};

class BlockStatement : public Statement {
public:
    VISITOR_ACCEPT

    const std::vector<std::shared_ptr<Statement>> statements;

    explicit BlockStatement(std::vector<std::shared_ptr<Statement>> statements)
        : statements(std::move(statements))
    {

    }
};

class VarDefStatement : public Statement {
public:
    VISITOR_ACCEPT

    const std::string identifier;
    const std::shared_ptr<Expression> expression;

    explicit VarDefStatement(std::string identifier, std::shared_ptr<Expression> expression = nullptr)
        : identifier(std::move(identifier))
        , expression(std::move(expression))
    {

    }
};


class FunctionCallExpression : public Expression {
public:
    const std::shared_ptr<Expression> expression;
    const std::vector<std::shared_ptr<Expression>> arguments;

    FunctionCallExpression(std::shared_ptr<Expression> expression, std::vector<std::shared_ptr<Expression>> arguments)
        : expression(std::move(expression))
        , arguments(std::move(arguments))
    {

    }
};

class FunctionDefExpression : public Expression {
public:
    const std::string name;
    const std::vector<std::string> args_list;
    const std::shared_ptr<BlockStatement> body;

    FunctionDefExpression(std::string name, std::vector<std::string> args_list, std::shared_ptr<BlockStatement> body)
        : name(std::move(name))
        , args_list(std::move(args_list))
        , body(std::move(body))
    {

    }
};

/*class MemberAccessExpression : public Expression {
public:
    const std::shared_ptr<Expression> expression;
    const std::string identifier;

    MemberAccessExpression(std::shared_ptr<Expression> expression, std::string identifier)
        : expression(std::move(expression))
        , identifier(std::move(identifier))
    {

    }

    [[nodiscard]]
    std::string ToString() const override {
        return expression->ToString() + "." + identifier;
    }
};*/

#undef VISITOR_ACCEPT

class ASTStringifier : public ASTVisitor {
public:
    void Visit(ASTNode& node) {
        str += "[ASTNode]";
    }

    void Visit(LiteralExpression& le) {
        str += TokenUtils::TokenToSource(le.type);
    }

    void Visit(NumberLiteralExpression& nle) {
        str += std::to_string(nle.number);
    }

    void Visit(StringLiteralExpression& sle) {
        str += "\"" + sle.string + "\"";
    }

    void Visit(IdentifierExpression& ie) {
        str += ie.identifier;
    }

    void Visit(BinaryExpression& be) {
        str += "(";
        be.left->Accept(*this);
        str += " " + TokenUtils::TokenToSource(be.op) + " ";
        be.right->Accept(*this);
        str += ")";
    }

    void Visit(UnaryExpression& ue) {
        str += "(";
        str += TokenUtils::TokenToSource(ue.op);
        ue.expression->Accept(*this);
        str += ")";
    }

    void Visit(PostfixExpression& pe) {
        str += "(";
        pe.expression->Accept(*this);
        str += TokenUtils::TokenToSource(pe.op);
        str += ")";
    }

    void Visit(ExpressionStatement& es) {
        str += std::string(indent, ' ');
        es.expression->Accept(*this);
        str += '\n';
    }

    void Visit(ReturnStatement& rs) {
        str += std::string(indent, ' ');
        str += "return ";
        rs.return_expression->Accept(*this);
        str += '\n';
    }

    void Visit(BlockStatement& bs) {
        str += std::string(indent, ' ');
        str += "{\n";
        indent += 4;
        for (auto& s : bs.statements) {
            s->Accept(*this);
        }
        indent -= 4;
        str += std::string(indent, ' ');
        str += "}\n";
    }

    void Visit(VarDefStatement& vds) {
        str += std::string(indent, ' ');
        str += "let " + vds.identifier + " = ";
        vds.expression->Accept(*this);
        str += '\n';
    }

    void Visit(FunctionCallExpression& fce) {
        fce.expression->Accept(*this);
        str += "(";
        for (size_t i = 0; i < fce.arguments.size(); ++i) {
            fce.arguments[i]->Accept(*this);
            if (i < fce.arguments.size() - 1) {
                str += ",";
            }
        }
        str += ")";
    }

    void Visit(FunctionDefExpression& fde) {

    }

    const std::string& ToString() const {
        return str;
    }

private:
    std::string str;
    size_t indent = 0;
};

}

#endif //NLANG_AST_HPP
