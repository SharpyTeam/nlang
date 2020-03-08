#pragma once

#include "utils/holder.hpp"
#include "token.hpp"

#include <utils/macro.hpp>

#include <string>
#include <memory>
#include <utility>

namespace nlang {

// base
class IASTNode;
class IExpression;
class IStatement;

// expressions
class NullExpression;
class BoolExpression;
class NumberExpression;
class StringExpression;
class IdentifierExpression;

class ParenthesizedExpression;
class PrefixExpression;
class PostfixExpression;
class BinaryExpression;

class OperatorDefinitionExpression;
class FunctionDefinitionExpression;
class FunctionCallExpression;
class SubscriptExpression;
class MemberAccessExpression;

class ClassDefinitionExpression;

// statements
class VariableDefinitionStatement;
class ExpressionStatement;
class BlockStatement;
class BranchStatement;
class WhileStatement;

class ReturnStatement;
class BreakStatement;
class ContinueStatement;


class IASTVisitor {
public:
    virtual void Visit(IASTNode&) = 0;
    virtual void Visit(IExpression&) = 0;
    virtual void Visit(IStatement&) = 0;

    virtual void Visit(NullExpression&) = 0;
    virtual void Visit(BoolExpression&) = 0;
    virtual void Visit(NumberExpression&) = 0;
    virtual void Visit(StringExpression&) = 0;
    virtual void Visit(IdentifierExpression&) = 0;

    virtual void Visit(ParenthesizedExpression&) = 0;
    virtual void Visit(PrefixExpression&) = 0;
    virtual void Visit(PostfixExpression&) = 0;
    virtual void Visit(BinaryExpression&) = 0;

    virtual void Visit(OperatorDefinitionExpression&) = 0;
    virtual void Visit(FunctionDefinitionExpression&) = 0;
    virtual void Visit(FunctionCallExpression&) = 0;
    virtual void Visit(SubscriptExpression&) = 0;
    virtual void Visit(MemberAccessExpression&) = 0;

    virtual void Visit(ClassDefinitionExpression&) = 0;

    virtual void Visit(VariableDefinitionStatement&) = 0;
    virtual void Visit(ExpressionStatement&) = 0;
    virtual void Visit(BlockStatement&) = 0;
    virtual void Visit(BranchStatement&) = 0;
    virtual void Visit(WhileStatement&) = 0;

    virtual void Visit(ReturnStatement&) = 0;
    virtual void Visit(BreakStatement&) = 0;
    virtual void Visit(ContinueStatement&) = 0;
};


#define VISITOR_ACCEPT                          \
virtual void Accept(IASTVisitor& visitor) {     \
    visitor.Visit(*this);                       \
}


class IASTNode {
public:
    VISITOR_ACCEPT

    IASTNode(const IASTNode&) = delete;
    IASTNode(IASTNode&&) = delete;
    IASTNode& operator=(const IASTNode&) = delete;
    IASTNode& operator=(IASTNode&&) = delete;

    virtual ~IASTNode() = 0;

protected:
    IASTNode() {};
};

IASTNode::~IASTNode() = default;


class IExpression : public IASTNode {
public:
    VISITOR_ACCEPT

    virtual ~IExpression() = 0;
};

IExpression::~IExpression() = default;


class IStatement : public IASTNode {
public:
    VISITOR_ACCEPT

    virtual ~IStatement() = 0;
};

IStatement::~IStatement() = default;


class NullExpression : public IExpression {
public:
    VISITOR_ACCEPT

    NullExpression() {}
};


class BoolExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const bool flag;

    explicit BoolExpression(bool flag) : flag(flag) {}
};


class NumberExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const double number;

    explicit NumberExpression(double number) : number(number) {}
};


class StringExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const std::string string;

    explicit StringExpression(std::string string) : string(std::move(string)) {}
};


class IdentifierExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const std::string identifier;

    explicit IdentifierExpression(std::string identifier) : identifier(std::move(identifier)) {}
};


class ParenthesizedExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;

    ParenthesizedExpression(Holder<IExpression>&& expression)
        : expression(std::move(expression))
    {

    }
};


class PrefixExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Token prefix;
    const Holder<IExpression> expression;

    PrefixExpression(Token prefix, Holder<IExpression>&& expression)
        : prefix(prefix)
        , expression(std::move(expression))
    {

    }
};


class PostfixExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Token postfix;
    const Holder<IExpression> expression;

    PostfixExpression(Token postfix, Holder<IExpression>&& expression)
        : postfix(postfix)
        , expression(std::move(expression))
    {

    }
};


class BinaryExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Token op;
    const Holder<IExpression> left;
    const Holder<IExpression> right;

    BinaryExpression(Token op, Holder<IExpression>&& left, Holder<IExpression>&& right)
        : op(op)
        , left(std::move(left))
        , right(std::move(right))
    {

    }
};


class VariableDefinitionStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const Holder<IdentifierExpression> name;
    const Holder<IExpression> value;
    const Holder<IdentifierExpression> type_hint;

    explicit VariableDefinitionStatement(
        Holder<IdentifierExpression>&& name,
        Holder<IExpression>&& value = nullptr,
        Holder<IdentifierExpression>&& type_hint = nullptr)

        : name(std::move(name))
        , value(std::move(value))
        , type_hint(std::move(type_hint))
    {

    }
};


class OperatorDefinitionExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IdentifierExpression> name;
    const Holder<IStatement> body;
    const std::vector<Holder<VariableDefinitionStatement>> arguments;
    const Holder<IdentifierExpression> type_hint;

    OperatorDefinitionExpression(
        Holder<IdentifierExpression>&& name,
        Holder<IStatement>&& body,
        std::vector<Holder<VariableDefinitionStatement>>&& arguments = std::vector<Holder<VariableDefinitionStatement>>(),
    Holder<IdentifierExpression>&& type_hint = nullptr)

    : name(std::move(name))
    , body(std::move(body))
    , arguments(std::move(arguments))
    , type_hint(std::move(type_hint))
    {
#ifdef NLANG_DEBUG
        bool th = false;
        for (auto& argument : arguments) {
            if (!th) {
                if (argument->type_hint.get()) {
                    th = true;
                }
            } else {
                NLANG_ASSERT(argument->type_hint.get() != nullptr);
            }
        }
#endif
    }
};


class FunctionDefinitionExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IdentifierExpression> name;
    const Holder<IStatement> body;
    const std::vector<Holder<VariableDefinitionStatement>> arguments;
    const Holder<IdentifierExpression> type_hint;

    FunctionDefinitionExpression(
        Holder<IdentifierExpression>&& name,
        Holder<IStatement>&& body,
        std::vector<Holder<VariableDefinitionStatement>>&& arguments = std::vector<Holder<VariableDefinitionStatement>>(),
        Holder<IdentifierExpression>&& type_hint = nullptr)

        : name(std::move(name))
        , body(std::move(body))
        , arguments(std::move(arguments))
        , type_hint(std::move(type_hint))
    {
#ifdef NLANG_DEBUG
        bool th = false;
        for (auto& argument : arguments) {
            if (!th) {
                if (argument->type_hint.get()) {
                    th = true;
                }
            } else {
                NLANG_ASSERT(argument->type_hint.get() != nullptr);
            }
        }
#endif
    }
};


class FunctionCallExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const std::vector<Holder<IExpression>> arguments;

    FunctionCallExpression(Holder<IExpression>&& expression, std::vector<Holder<IExpression>>&& arguments)
        : expression(std::move(expression))
        , arguments(std::move(arguments))
    {

    }
};


class SubscriptExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const std::vector<Holder<IExpression>> arguments;

    SubscriptExpression(Holder<IExpression>&& expression, std::vector<Holder<IExpression>>&& arguments)
        : expression(std::move(expression))
        , arguments(std::move(arguments))
    {

    }
};


class MemberAccessExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const Holder<IdentifierExpression> name;

    MemberAccessExpression(Holder<IExpression>&& expression, Holder<IdentifierExpression>&& name)
        : expression(std::move(expression))
        , name(std::move(name))
    {

    }
};


class ClassDefinitionExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IdentifierExpression> name;
    const Holder<FunctionDefinitionExpression> constructor;
    const std::vector<Holder<VariableDefinitionStatement>> fields;
    const std::vector<Holder<FunctionDefinitionExpression>> methods;

    ClassDefinitionExpression(
        Holder<IdentifierExpression>&& name,
        Holder<FunctionDefinitionExpression>&& constructor,
        std::vector<Holder<VariableDefinitionStatement>>&& fields,
        std::vector<Holder<FunctionDefinitionExpression>>&& methods)

        : name(std::move(name))
        , constructor(std::move(constructor))
        , fields(std::move(fields))
        , methods(std::move(methods))
    {

    }
};


class ExpressionStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;

    explicit ExpressionStatement(Holder<IExpression>&& expression)
        : expression(std::move(expression))
    {

    }
};


class BlockStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const std::vector<Holder<IStatement>> statements;

    explicit BlockStatement(std::vector<Holder<IStatement>>&& statements)
        : statements(std::move(statements))
    {

    }
};


class BranchStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const std::vector<std::pair<Holder<IExpression>, Holder<IStatement>>> branches;

    explicit BranchStatement(std::vector<std::pair<Holder<IExpression>, Holder<IStatement>>>&& branches)
        : branches(std::move(branches))
    {

    }
};


class WhileStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> condition;
    const Holder<IStatement> body;

    explicit WhileStatement(Holder<IExpression>&& condition, Holder<IStatement>&& body)
        : condition(std::move(condition))
        , body(std::move(body))
    {

    }
};


class ReturnStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;

    explicit ReturnStatement(Holder<IExpression>&& expression = nullptr)
        : expression(std::move(expression))
    {

    }
};


class BreakStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;

    explicit BreakStatement(Holder<IExpression>&& expression = nullptr)
        : expression(std::move(expression))
    {

    }
};


class ContinueStatement : public IStatement {
public:
    VISITOR_ACCEPT

    ContinueStatement() {}
};


#undef VISITOR_ACCEPT


class ASTPrinter : public IASTVisitor {
public:
    std::string text;
    std::string indent = "";

    void Visit(IASTNode&) override {};
    void Visit(IExpression&) override {};
    void Visit(IStatement&) override {};

    void Visit(NullExpression&) override {
        text += "null";
    };

    void Visit(BoolExpression& e) override {
        text += (e.flag ? "true" : "false");
    }

    void Visit(NumberExpression& e) override {
        text += std::to_string(e.number);
    };

    void Visit(StringExpression& e) override {
        text += "'" + e.string + "'";
    }

    void Visit(IdentifierExpression& e) override {
        text += e.identifier;
    }

    void Visit(ParenthesizedExpression& e) override {
        text += "(";
        e.expression->Accept(*this);
        text += ")";
    }

    void Visit(PrefixExpression& e) override {
        text += TokenUtils::GetTokenText(e.prefix);
        e.expression->Accept(*this);
    }

    void Visit(PostfixExpression& e) override {
        e.expression->Accept(*this);
        text += TokenUtils::GetTokenText(e.postfix);
    }

    void Visit(BinaryExpression&e) override {
        e.left->Accept(*this);
        text += " " + TokenUtils::GetTokenText(e.op) + " ";
        e.right->Accept(*this);
    }

    void Visit(OperatorDefinitionExpression& e) override {
        // TODO
    }

    void Visit(FunctionDefinitionExpression& e) override {
        text += "fn ";
        if (e.name) {
            e.name->Accept(*this);
        }
        text += "(";
        auto visit_arg = [&](VariableDefinitionStatement& s) {
            s.name->Accept(*this);
            if (s.type_hint) {
                text += ": ";
                s.type_hint->Accept(*this);
            }
            if (s.value) {
                text += " = ";
                s.value->Accept(*this);
            }
        };
        for (size_t i = 0; i < e.arguments.size(); ++i) {
            visit_arg(*e.arguments[i]);
            if (i + 1 != e.arguments.size()) {
                text += ", ";
            }
        }
        text += ")";
        if (e.type_hint) {
            text += ": ";
            e.type_hint->Accept(*this);
        }
        text += " ";
        e.body->Accept(*this);
    }

    void Visit(FunctionCallExpression& e) override {
        e.expression->Accept(*this);
        text += "(";
        for (size_t i = 0; i < e.arguments.size(); ++i) {
            e.arguments[i]->Accept(*this);
            if (i + 1 != e.arguments.size()) {
                text += ", ";
            }
        }
        text += ")";
    }

    void Visit(SubscriptExpression& e) override {
        e.expression->Accept(*this);
        text += "[";
        for (size_t i = 0; i < e.arguments.size(); ++i) {
            e.arguments[i]->Accept(*this);
            if (i + 1 != e.arguments.size()) {
                text += ", ";
            }
        }
        text += "]";
    }

    void Visit(MemberAccessExpression& e) override {
        e.expression->Accept(*this);
        text += ".";
        e.name->Accept(*this);
    }

    void Visit(ClassDefinitionExpression&) override {
        // TODO
    }

    void Visit(VariableDefinitionStatement& s) override {
        text += "let ";
        s.name->Accept(*this);
        if (s.type_hint) {
            text += ": ";
            s.type_hint->Accept(*this);
        }
        if (s.value) {
            text += " = ";
            s.value->Accept(*this);
        }
        text += "\n" + indent;
    }

    void Visit(ExpressionStatement& s) override {
        s.expression->Accept(*this);
        text += "\n" + indent;
    }

    void Visit(BlockStatement& s) override {
        text += "{\n";
        indent.resize(indent.size() + 4, ' ');
        text += indent;
        for (auto& ss : s.statements) {
            ss->Accept(*this);
        }
        text.resize(text.size() - 4);
        indent.resize(indent.size() - 4);
        text += "}\n" + indent;
    }

    void Visit(BranchStatement& s) override {
        for (size_t i = 0; i < s.branches.size(); ++i) {
            auto& [ee, ss] = s.branches[i];
            if (ee) {
                text += "if (";
                ee->Accept(*this);
                text += ") ";
            }
            ss->Accept(*this);
            if (i + 1 != s.branches.size()) {
                text.resize(text.find_last_of('}') + 1);
                text += " else ";
            }
        }
    }

    void Visit(WhileStatement& s) override {
        text += "while (";
        s.condition->Accept(*this);
        text += ") ";
        s.body->Accept(*this);
    }

    void Visit(ReturnStatement& s) override {
        text += "return";
        if (s.expression) {
            text += " ";
            s.expression->Accept(*this);
        }
        text += "\n" + indent;
    }

    void Visit(BreakStatement& s) override {
        text += "break";
        if (s.expression) {
            text += " ";
            s.expression->Accept(*this);
        }
        text += "\n" + indent;
    }

    void Visit(ContinueStatement&) override {
        text += "return\n" + indent;
    }
};

}
