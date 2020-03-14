#pragma once

#include "utils/holder.hpp"
#include "token.hpp"

#include <utils/macro.hpp>

#include <string>
#include <memory>
#include <utility>
#include <cctype>

namespace nlang {

// base
class IASTNode;
class IExpression;
class IStatement;
class ILiteral;

// literals
class NullLiteral;
class BoolLiteral;
class NumberLiteral;
class StringLiteral;
class IdentifierLiteral;

// helpers
class TypeHint;
class DefaultValue;
class ElseStatementPart;
class ArgumentDefinitionStatementPart;

// expressions
class LiteralExpression;
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
class IfElseStatement;
class WhileStatement;

class ReturnStatement;
class BreakStatement;
class ContinueStatement;


class IASTVisitor {
public:
    virtual void Visit(IASTNode&) = 0;
    virtual void Visit(IExpression&) = 0;
    virtual void Visit(IStatement&) = 0;

    virtual void Visit(NullLiteral&) = 0;
    virtual void Visit(BoolLiteral&) = 0;
    virtual void Visit(NumberLiteral&) = 0;
    virtual void Visit(StringLiteral&) = 0;
    virtual void Visit(IdentifierLiteral&) = 0;

    virtual void Visit(TypeHint&) = 0;
    virtual void Visit(DefaultValue&) = 0;
    virtual void Visit(ElseStatementPart&) = 0;
    virtual void Visit(ArgumentDefinitionStatementPart&) = 0;

    virtual void Visit(LiteralExpression&) = 0;
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
    virtual void Visit(IfElseStatement&) = 0;
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


class ILiteral : public IASTNode {
public:
    VISITOR_ACCEPT

    virtual ~ILiteral() = 0;
};

ILiteral::~ILiteral() = default;


class NullLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    const TokenInstance token;

    NullLiteral(TokenInstance&& token)
        : token(std::move(token))
    {}
};


class BoolLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    const TokenInstance token;
    const bool flag;

    explicit BoolLiteral(TokenInstance&& token)
        : token(std::move(token))
        , flag(this->token.token == Token::THE_TRUE)
    {}
};


class NumberLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    const TokenInstance token;
    const double number;

    explicit NumberLiteral(TokenInstance&& token)
        : token(std::move(token))
        , number(std::stod(this->token.text))
    {}
};


class StringLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    const TokenInstance token;
    const std::string string;

    explicit StringLiteral(TokenInstance&& token)
        : token(std::move(token))
        , string(this->token.text)
    {}
};


class IdentifierLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    const TokenInstance token;
    const std::string& identifier;

    explicit IdentifierLiteral(TokenInstance&& token)
        : token(std::move(token))
        , identifier(this->token.text)
    {}
};


class TypeHint : public IASTNode {
public:
    VISITOR_ACCEPT

    const TokenInstance colon;
    const Holder<IdentifierLiteral> name;

    TypeHint(TokenInstance&& colon, Holder<IdentifierLiteral>&& name)
        : colon(std::move(colon))
        , name(std::move(name))
    {}
};


class DefaultValue : public IASTNode {
public:
    VISITOR_ACCEPT

    const TokenInstance assignment;
    const Holder<IExpression> value;

    DefaultValue(TokenInstance&& assignment, Holder<IExpression>&& value)
        : assignment(std::move(assignment))
        , value(std::move(value))
    {}
};


class LiteralExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<ILiteral> literal;

    LiteralExpression(Holder<ILiteral>&& literal)
        : literal(std::move(literal))
    {}
};


class ParenthesizedExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const TokenInstance left_par;
    const Holder<IExpression> expression;
    const TokenInstance right_par;

    ParenthesizedExpression(TokenInstance&& left_par, Holder<IExpression>&& expression, TokenInstance&& right_par)
        : left_par(std::move(left_par))
        , expression(std::move(expression))
        , right_par(std::move(right_par))
    {}
};


class PrefixExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const TokenInstance prefix;
    const Holder<IExpression> expression;

    PrefixExpression(TokenInstance&& prefix, Holder<IExpression>&& expression)
        : prefix(std::move(prefix))
        , expression(std::move(expression))
    {}
};


class PostfixExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const TokenInstance postfix;

    PostfixExpression(Holder<IExpression>&& expression, TokenInstance&& postfix)
        : expression(std::move(expression))
        , postfix(std::move(postfix))
    {}
};


class BinaryExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> left;
    const TokenInstance op;
    const Holder<IExpression> right;

    BinaryExpression(Holder<IExpression>&& left, TokenInstance&& op, Holder<IExpression>&& right)
        : left(std::move(left))
        , op(std::move(op))
        , right(std::move(right))
    {}
};


class VariableDefinitionStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance let;
    const Holder<IdentifierLiteral> name;
    const Holder<TypeHint> type_hint;
    const Holder<DefaultValue> default_value;

    explicit VariableDefinitionStatement(
        TokenInstance&& let,
        Holder<IdentifierLiteral>&& name,
        Holder<TypeHint>&& type_hint,
        Holder<DefaultValue>&& default_value)

        : let(std::move(let))
        , name(std::move(name))
        , type_hint(std::move(type_hint))
        , default_value(std::move(default_value))
    {}
};


class ArgumentDefinitionStatementPart : public IASTNode {
public:
    VISITOR_ACCEPT

    const Holder<IdentifierLiteral> name;
    const Holder<TypeHint> type_hint;
    const Holder<DefaultValue> default_value;

    explicit ArgumentDefinitionStatementPart(
        Holder<IdentifierLiteral>&& name,
        Holder<TypeHint>&& type_hint,
        Holder<DefaultValue>&& default_value)

        : name(std::move(name))
        , type_hint(std::move(type_hint))
        , default_value(std::move(default_value))
    {}
};


/*class OperatorDefinitionExpression : public IExpression {
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
};*/


class FunctionDefinitionExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const TokenInstance fn;
    const Holder<IdentifierLiteral> name;
    const TokenInstance left_par;
    const std::vector<Holder<ArgumentDefinitionStatementPart>> arguments;
    const TokenInstance right_par;
    const Holder<TypeHint> type_hint;
    const Holder<IStatement> body;

    FunctionDefinitionExpression(
        TokenInstance&& fn,
        Holder<IdentifierLiteral>&& name,
        TokenInstance&& left_par,
        std::vector<Holder<ArgumentDefinitionStatementPart>>&& arguments,
        TokenInstance&& right_par,
        Holder<TypeHint>&& type_hint,
        Holder<IStatement>&& body)

        : fn(std::move(fn))
        , name(std::move(name))
        , left_par(std::move(left_par))
        , arguments(std::move(arguments))
        , right_par(std::move(right_par))
        , type_hint(std::move(type_hint))
        , body(std::move(body))
    {}
};


class FunctionCallExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const TokenInstance left_par;
    const std::vector<Holder<IExpression>> arguments;
    const TokenInstance right_par;

    FunctionCallExpression(Holder<IExpression>&& expression, TokenInstance&& left_par, std::vector<Holder<IExpression>>&& arguments, TokenInstance&& right_par)
        : expression(std::move(expression))
        , left_par(std::move(left_par))
        , arguments(std::move(arguments))
        , right_par(std::move(right_par))
    {}
};


class SubscriptExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const TokenInstance left_bracket;
    const std::vector<Holder<IExpression>> arguments;
    const TokenInstance right_bracket;

    SubscriptExpression(Holder<IExpression>&& expression, TokenInstance&& left_bracket, std::vector<Holder<IExpression>>&& arguments, TokenInstance&& right_bracket)
        : expression(std::move(expression))
        , left_bracket(std::move(left_bracket))
        , arguments(std::move(arguments))
        , right_bracket(std::move(right_bracket))
    {}
};


class MemberAccessExpression : public IExpression {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;
    const TokenInstance dot;
    const Holder<IdentifierLiteral> name;

    MemberAccessExpression(Holder<IExpression>&& expression, TokenInstance&& dot, Holder<IdentifierLiteral>&& name)
        : expression(std::move(expression))
        , dot(std::move(dot))
        , name(std::move(name))
    {}
};


/*class ClassDefinitionExpression : public IExpression {
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
};*/


class ExpressionStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const Holder<IExpression> expression;

    explicit ExpressionStatement(Holder<IExpression>&& expression)
        : expression(std::move(expression))
    {}
};


class BlockStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance left_brace;
    const std::vector<Holder<IStatement>> statements;
    const TokenInstance right_brace;

    explicit BlockStatement(TokenInstance&& right_brace, std::vector<Holder<IStatement>>&& statements, TokenInstance&& left_brace)
        : left_brace(std::move(left_brace))
        , statements(std::move(statements))
        , right_brace(std::move(right_brace))
    {}
};


class ElseStatementPart : public IASTNode {
public:
    VISITOR_ACCEPT

    const TokenInstance else_token;
    const Holder<IStatement> body;

    ElseStatementPart(TokenInstance&& else_token, Holder<IStatement>&& body)
        : else_token(std::move(else_token))
        , body(std::move(body))
    {}
};

class IfElseStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance if_token;
    const TokenInstance left_par;
    const Holder<IExpression> condition;
    const TokenInstance right_par;
    const Holder<IStatement> body;
    const Holder<ElseStatementPart> else_branch;

    explicit IfElseStatement(
        TokenInstance&& if_token,
        TokenInstance&& left_par,
        Holder<IExpression>&& condition,
        TokenInstance&& right_par,
        Holder<IStatement>&& body,
        Holder<ElseStatementPart>&& else_branch)

        : if_token(std::move(if_token))
        , left_par(std::move(left_par))
        , condition(std::move(condition))
        , right_par(std::move(right_par))
        , body(std::move(body))
        , else_branch(std::move(else_branch))
    {}
};


class WhileStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance while_token;
    const TokenInstance left_par;
    const Holder<IExpression> condition;
    const TokenInstance right_par;
    const Holder<IStatement> body;

    explicit WhileStatement(
        TokenInstance&& while_token,
        TokenInstance&& left_par,
        Holder<IExpression>&& condition,
        TokenInstance&& right_par,
        Holder<IStatement>&& body)

        : while_token(std::move(while_token))
        , left_par(std::move(left_par))
        , condition(std::move(condition))
        , right_par(std::move(right_par))
        , body(std::move(body))
    {}
};


class ReturnStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance return_token;
    const Holder<IExpression> expression;

    explicit ReturnStatement(TokenInstance&& return_token, Holder<IExpression>&& expression)
        : return_token(std::move(return_token))
        , expression(std::move(expression))
    {}
};


class BreakStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance break_token;
    const Holder<IExpression> expression;

    explicit BreakStatement(TokenInstance&& break_token, Holder<IExpression>&& expression)
        : break_token(std::move(break_token))
        , expression(std::move(expression))
    {}
};


class ContinueStatement : public IStatement {
public:
    VISITOR_ACCEPT

    const TokenInstance continue_token;

    ContinueStatement(TokenInstance&& continue_token)
        : continue_token(std::move(continue_token))
    {}
};


#undef VISITOR_ACCEPT


class ASTStringifier : protected IASTVisitor {
public:
    std::string Stringify(const IASTNode& node) const {
        text.clear();
        indent.clear();
        const_cast<IASTNode&>(node).Accept(const_cast<ASTStringifier&>(*this));

        std::string text_trimmed;

        size_t pos = 0;
        while (pos < text.size()) {
            size_t newline_shift = text.find_first_of('\n', pos);
            newline_shift = (newline_shift == std::string::npos) ? text.size() : newline_shift - pos;
            std::string substr = text.substr(pos, newline_shift);
            for (char c : substr) {
                if (!isspace(c)) {
                    if (!text_trimmed.empty()) {
                        text_trimmed += "\n";
                    }
                    for (size_t i = substr.size(); i > 0; --i) {
                        if (!isspace(substr[i - 1])) {
                            substr.resize(i);
                            break;
                        }
                    }
                    text_trimmed += substr;
                    break;
                }
            }
            pos += newline_shift + 1;
        }

        return text_trimmed;
    }

private:
    mutable std::string text;
    mutable std::string indent = "";

    void Visit(IASTNode&) override {};
    void Visit(IExpression&) override {};
    void Visit(IStatement&) override {};

    void Visit(NullLiteral&) override {
        text += "null";
    };

    void Visit(BoolLiteral& e) override {
        text += (e.flag ? "true" : "false");
    }

    void Visit(NumberLiteral& e) override {
        text += std::to_string(e.number);
    };

    void Visit(StringLiteral& e) override {
        text += "'" + e.string + "'";
    }

    void Visit(IdentifierLiteral& e) override {
        text += e.identifier;
    }

    void Visit(TypeHint& t) override {
        text += ": ";
        t.name->Accept(*this);
    }

    void Visit(DefaultValue& v) override {
        text += " = ";
        v.value->Accept(*this);
    }

    void Visit(ElseStatementPart& s) override {
        text += "else ";
        s.body->Accept(*this);
    }

    void Visit(ArgumentDefinitionStatementPart& s) override {
        s.name->Accept(*this);
        if (s.type_hint) {
            s.type_hint->Accept(*this);
        }
        if (s.default_value) {
            s.default_value->Accept(*this);
        }
    }

    void Visit(LiteralExpression& e) override {
        e.literal->Accept(*this);
    }

    void Visit(ParenthesizedExpression& e) override {
        text += "(";
        e.expression->Accept(*this);
        text += ")";
    }

    void Visit(PrefixExpression& e) override {
        text += TokenUtils::GetTokenText(e.prefix.token);
        e.expression->Accept(*this);
    }

    void Visit(PostfixExpression& e) override {
        e.expression->Accept(*this);
        text += TokenUtils::GetTokenText(e.postfix.token);
    }

    void Visit(BinaryExpression&e) override {
        e.left->Accept(*this);
        text += " " + TokenUtils::GetTokenText(e.op.token) + " ";
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
        for (size_t i = 0; i < e.arguments.size(); ++i) {
            e.arguments[i]->Accept(*this);
            if (i + 1 != e.arguments.size()) {
                text += ", ";
            }
        }
        text += ")";
        if (e.type_hint) {
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
            s.type_hint->Accept(*this);
        }
        if (s.default_value) {
            s.default_value->Accept(*this);
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

    void Visit(IfElseStatement& s) override {
        text += "if (";
        s.condition->Accept(*this);
        text += ") ";
        s.body->Accept(*this);
        if (s.else_branch) {
            text.resize(text.find_last_of('}') + 1);
            text += " ";
            s.else_branch->Accept(*this);
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
