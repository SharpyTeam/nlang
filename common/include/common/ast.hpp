#pragma once

#include <common/token.hpp>

#include <utils/macro.hpp>
#include <utils/pointers.hpp>
#include <utils/strings.hpp>

#include <utility>
#include <cctype>
#include <vector>

namespace nlang::ast {

// base
class INode;
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
class FunctionDefinitionStatement;
class VariableDefinitionStatement;
class ExpressionStatement;
class BlockStatement;
class IfElseStatement;
class WhileStatement;

class ReturnStatement;
class BreakStatement;
class ContinueStatement;

// top level
class Module;


class IASTVisitor {
public:
    virtual void Visit(INode&) = 0;
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

    virtual void Visit(FunctionDefinitionStatement&) = 0;
    virtual void Visit(VariableDefinitionStatement&) = 0;
    virtual void Visit(ExpressionStatement&) = 0;
    virtual void Visit(BlockStatement&) = 0;
    virtual void Visit(IfElseStatement&) = 0;
    virtual void Visit(WhileStatement&) = 0;

    virtual void Visit(ReturnStatement&) = 0;
    virtual void Visit(BreakStatement&) = 0;
    virtual void Visit(ContinueStatement&) = 0;

    virtual void Visit(Module&) = 0;
};


#define VISITOR_ACCEPT                          \
virtual void Accept(IASTVisitor& visitor) {     \
    visitor.Visit(*this);                       \
}


class INode : public IntrusivePtrRefCounter {
public:
    VISITOR_ACCEPT

    class IMeta : public IntrusivePtrRefCounter {
    public:
        virtual ~IMeta() = 0;
    };

    IntrusivePtr<IMeta> meta;

    INode(const INode&) = delete;
    INode(INode&&) = delete;
    INode& operator=(const INode&) = delete;
    INode& operator=(INode&&) = delete;

    virtual ~INode() = 0;

protected:
    INode() {};
};

INode::IMeta::~IMeta() = default;
INode::~INode() = default;


class IExpression : public INode {
public:
    VISITOR_ACCEPT

    virtual ~IExpression() = 0;
};

IExpression::~IExpression() = default;


class IStatement : public INode {
public:
    VISITOR_ACCEPT

    virtual ~IStatement() = 0;
};

IStatement::~IStatement() = default;


class ILiteral : public INode {
public:
    VISITOR_ACCEPT

    virtual ~ILiteral() = 0;
};

ILiteral::~ILiteral() = default;


class NullLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    TokenInstance token;

    NullLiteral(TokenInstance&& token)
        : token(std::move(token))
    {}
};


class BoolLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    TokenInstance token;
    bool flag;

    explicit BoolLiteral(TokenInstance&& token)
        : token(std::move(token))
        , flag(this->token.token == Token::THE_TRUE)
    {}
};


class NumberLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    TokenInstance token;
    double number;

    explicit NumberLiteral(TokenInstance&& token)
        : token(std::move(token))
    {
        number = std::stod(token.text.GetStdStr());
    }
};


class StringLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    TokenInstance token;
    UString string;

    explicit StringLiteral(TokenInstance&& token)
        : token(std::move(token))
        , string(UString(this->token.text, 1, this->token.text.GetLength() - 2).Unescape())
    {

    }
};


class IdentifierLiteral : public ILiteral {
public:
    VISITOR_ACCEPT

    TokenInstance token;
    UString identifier;

    explicit IdentifierLiteral(TokenInstance&& token)
        : token(std::move(token))
        , identifier(this->token.text.SubStringView())
    {}
};


class TypeHint : public INode {
public:
    VISITOR_ACCEPT

    TokenInstance colon;
    UniquePtr<IdentifierLiteral> name;

    TypeHint(TokenInstance&& colon, UniquePtr<IdentifierLiteral>&& name)
        : colon(std::move(colon))
        , name(std::move(name))
    {}
};


class DefaultValue : public INode {
public:
    VISITOR_ACCEPT

    TokenInstance assignment;
    UniquePtr<IExpression> value;

    DefaultValue(TokenInstance&& assignment, UniquePtr<IExpression>&& value)
        : assignment(std::move(assignment))
        , value(std::move(value))
    {}
};


class LiteralExpression : public IExpression {
public:
    VISITOR_ACCEPT

    UniquePtr<ILiteral> literal;

    LiteralExpression(UniquePtr<ILiteral>&& literal)
        : literal(std::move(literal))
    {}
};


class ParenthesizedExpression : public IExpression {
public:
    VISITOR_ACCEPT

    TokenInstance left_par;
    UniquePtr<IExpression> expression;
    TokenInstance right_par;

    ParenthesizedExpression(TokenInstance&& left_par, UniquePtr<IExpression>&& expression, TokenInstance&& right_par)
        : left_par(std::move(left_par))
        , expression(std::move(expression))
        , right_par(std::move(right_par))
    {}
};


class PrefixExpression : public IExpression {
public:
    VISITOR_ACCEPT

    TokenInstance prefix;
    UniquePtr<IExpression> expression;

    PrefixExpression(TokenInstance&& prefix, UniquePtr<IExpression>&& expression)
        : prefix(std::move(prefix))
        , expression(std::move(expression))
    {}
};


class PostfixExpression : public IExpression {
public:
    VISITOR_ACCEPT

    UniquePtr<IExpression> expression;
    TokenInstance postfix;

    PostfixExpression(UniquePtr<IExpression>&& expression, TokenInstance&& postfix)
        : expression(std::move(expression))
        , postfix(std::move(postfix))
    {}
};


class BinaryExpression : public IExpression {
public:
    VISITOR_ACCEPT

    UniquePtr<IExpression> left;
    TokenInstance op;
    UniquePtr<IExpression> right;

    BinaryExpression(UniquePtr<IExpression>&& left, TokenInstance&& op, UniquePtr<IExpression>&& right)
        : left(std::move(left))
        , op(std::move(op))
        , right(std::move(right))
    {}
};


class VariableDefinitionStatement : public IStatement {
public:
    VISITOR_ACCEPT

    TokenInstance let;
    UniquePtr<IdentifierLiteral> name;
    UniquePtr<TypeHint> type_hint;
    UniquePtr<DefaultValue> default_value;

    explicit VariableDefinitionStatement(
        TokenInstance&& let,
        UniquePtr<IdentifierLiteral>&& name,
        UniquePtr<TypeHint>&& type_hint,
        UniquePtr<DefaultValue>&& default_value)

        : let(std::move(let))
        , name(std::move(name))
        , type_hint(std::move(type_hint))
        , default_value(std::move(default_value))
    {}
};


class ArgumentDefinitionStatementPart : public INode {
public:
    VISITOR_ACCEPT

    UniquePtr<IdentifierLiteral> name;
    UniquePtr<TypeHint> type_hint;
    UniquePtr<DefaultValue> default_value;
    int32_t index = 0;

    explicit ArgumentDefinitionStatementPart(
        UniquePtr<IdentifierLiteral>&& name,
        UniquePtr<TypeHint>&& type_hint,
        UniquePtr<DefaultValue>&& default_value)

        : name(std::move(name))
        , type_hint(std::move(type_hint))
        , default_value(std::move(default_value))
    {}
};


/*class OperatorDefinitionExpression : public IExpression {
public:
    VISITOR_ACCEPT

    Holder<IdentifierExpression> name;
    Holder<IStatement> body;
    std::vector<Holder<VariableDefinitionStatement>> arguments;
    Holder<IdentifierExpression> type_hint;

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

    TokenInstance fn;
    UniquePtr<IdentifierLiteral> name;
    TokenInstance left_par;
    std::vector<UniquePtr<ArgumentDefinitionStatementPart>> arguments;
    TokenInstance right_par;
    UniquePtr<TypeHint> type_hint;
    UniquePtr<IStatement> body;

    FunctionDefinitionExpression(
        TokenInstance&& fn,
        UniquePtr<IdentifierLiteral>&& name,
        TokenInstance&& left_par,
        std::vector<UniquePtr<ArgumentDefinitionStatementPart>>&& arguments,
        TokenInstance&& right_par,
        UniquePtr<TypeHint>&& type_hint,
        UniquePtr<IStatement>&& body)

        : fn(std::move(fn))
        , name(std::move(name))
        , left_par(std::move(left_par))
        , arguments(std::move(arguments))
        , right_par(std::move(right_par))
        , type_hint(std::move(type_hint))
        , body(std::move(body))
    {}
};


class FunctionDefinitionStatement : public IStatement {
public:
    VISITOR_ACCEPT

    TokenInstance fn;
    UniquePtr<IdentifierLiteral> name;
    TokenInstance left_par;
    std::vector<UniquePtr<ArgumentDefinitionStatementPart>> arguments;
    TokenInstance right_par;
    UniquePtr<TypeHint> type_hint;
    UniquePtr<IStatement> body;

    FunctionDefinitionStatement(
        TokenInstance&& fn,
        UniquePtr<IdentifierLiteral>&& name,
        TokenInstance&& left_par,
        std::vector<UniquePtr<ArgumentDefinitionStatementPart>>&& arguments,
        TokenInstance&& right_par,
        UniquePtr<TypeHint>&& type_hint,
        UniquePtr<IStatement>&& body)

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

    UniquePtr<IExpression> expression;
    TokenInstance left_par;
    std::vector<UniquePtr<IExpression>> arguments;
    TokenInstance right_par;

    FunctionCallExpression(UniquePtr<IExpression>&& expression, TokenInstance&& left_par, std::vector<UniquePtr<IExpression>>&& arguments, TokenInstance&& right_par)
        : expression(std::move(expression))
        , left_par(std::move(left_par))
        , arguments(std::move(arguments))
        , right_par(std::move(right_par))
    {}
};


class SubscriptExpression : public IExpression {
public:
    VISITOR_ACCEPT

    UniquePtr<IExpression> expression;
    TokenInstance left_bracket;
    std::vector<UniquePtr<IExpression>> arguments;
    TokenInstance right_bracket;

    SubscriptExpression(UniquePtr<IExpression>&& expression, TokenInstance&& left_bracket, std::vector<UniquePtr<IExpression>>&& arguments, TokenInstance&& right_bracket)
        : expression(std::move(expression))
        , left_bracket(std::move(left_bracket))
        , arguments(std::move(arguments))
        , right_bracket(std::move(right_bracket))
    {}
};


class MemberAccessExpression : public IExpression {
public:
    VISITOR_ACCEPT

    UniquePtr<IExpression> expression;
    TokenInstance dot;
    UniquePtr<IdentifierLiteral> name;

    MemberAccessExpression(UniquePtr<IExpression>&& expression, TokenInstance&& dot, UniquePtr<IdentifierLiteral>&& name)
        : expression(std::move(expression))
        , dot(std::move(dot))
        , name(std::move(name))
    {}
};


/*class ClassDefinitionExpression : public IExpression {
public:
    VISITOR_ACCEPT

    Holder<IdentifierExpression> name;
    Holder<FunctionDefinitionExpression> constructor;
    std::vector<Holder<VariableDefinitionStatement>> fields;
    std::vector<Holder<FunctionDefinitionExpression>> methods;

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

    UniquePtr<IExpression> expression;

    explicit ExpressionStatement(UniquePtr<IExpression>&& expression)
        : expression(std::move(expression))
    {}
};


class BlockStatement : public IStatement {
public:
    VISITOR_ACCEPT

    TokenInstance left_brace;
    std::vector<UniquePtr<IStatement>> statements;
    TokenInstance right_brace;

    explicit BlockStatement(TokenInstance&& right_brace, std::vector<UniquePtr<IStatement>>&& statements, TokenInstance&& left_brace)
        : left_brace(std::move(left_brace))
        , statements(std::move(statements))
        , right_brace(std::move(right_brace))
    {}
};


class ElseStatementPart : public INode {
public:
    VISITOR_ACCEPT

    TokenInstance else_token;
    UniquePtr<IStatement> body;

    ElseStatementPart(TokenInstance&& else_token, UniquePtr<IStatement>&& body)
        : else_token(std::move(else_token))
        , body(std::move(body))
    {}
};

class IfElseStatement : public IStatement {
public:
    VISITOR_ACCEPT

    TokenInstance if_token;
    TokenInstance left_par;
    UniquePtr<IExpression> condition;
    TokenInstance right_par;
    UniquePtr<IStatement> body;
    UniquePtr<ElseStatementPart> else_branch;

    explicit IfElseStatement(
        TokenInstance&& if_token,
        TokenInstance&& left_par,
        UniquePtr<IExpression>&& condition,
        TokenInstance&& right_par,
        UniquePtr<IStatement>&& body,
        UniquePtr<ElseStatementPart>&& else_branch)

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

    TokenInstance while_token;
    TokenInstance left_par;
    UniquePtr<IExpression> condition;
    TokenInstance right_par;
    UniquePtr<IStatement> body;

    explicit WhileStatement(
        TokenInstance&& while_token,
        TokenInstance&& left_par,
        UniquePtr<IExpression>&& condition,
        TokenInstance&& right_par,
        UniquePtr<IStatement>&& body)

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

    TokenInstance return_token;
    UniquePtr<IExpression> expression;

    explicit ReturnStatement(TokenInstance&& return_token, UniquePtr<IExpression>&& expression)
        : return_token(std::move(return_token))
        , expression(std::move(expression))
    {}
};


class BreakStatement : public IStatement {
public:
    VISITOR_ACCEPT

    TokenInstance break_token;
    UniquePtr<IExpression> expression;

    explicit BreakStatement(TokenInstance&& break_token, UniquePtr<IExpression>&& expression)
        : break_token(std::move(break_token))
        , expression(std::move(expression))
    {}
};


class ContinueStatement : public IStatement {
public:
    VISITOR_ACCEPT

    TokenInstance continue_token;

    ContinueStatement(TokenInstance&& continue_token)
        : continue_token(std::move(continue_token))
    {}
};


class Module : public INode {
public:
    VISITOR_ACCEPT

    std::vector<UniquePtr<IStatement>> statements;

    explicit Module(std::vector<UniquePtr<IStatement>>&& statements)
        : statements(std::move(statements))
    {}
};


#undef VISITOR_ACCEPT


class ASTStringifier : protected IASTVisitor {
public:
    UString Stringify(const INode& node) const {
        text.Clear();
        indent.Clear();
        const_cast<INode&>(node).Accept(const_cast<ASTStringifier&>(*this));
        return text.Trim();
    }

private:
    mutable UString text;
    mutable UString indent = "";

    void Visit(INode&) override {};
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
        text += "'" + e.string.GetStdStr() + "'";
    }

    void Visit(IdentifierLiteral& e) override {
        text += e.identifier.GetStdStr();
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

    void Visit(FunctionDefinitionStatement& s) override {
        text += "fn ";
        if (s.name) {
            s.name->Accept(*this);
        }
        text += "(";
        for (size_t i = 0; i < s.arguments.size(); ++i) {
            s.arguments[i]->Accept(*this);
            if (i + 1 != s.arguments.size()) {
                text += ", ";
            }
        }
        text += ")";
        if (s.type_hint) {
            s.type_hint->Accept(*this);
        }
        text += " ";
        s.body->Accept(*this);
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
        indent += "    ";
        text += indent;
        for (auto& ss : s.statements) {
            ss->Accept(*this);
        }
        text.Truncate(text.GetLength() - 4);
        indent.Truncate(indent.GetLength() - 4);
        text += "}\n" + indent;
    }

    void Visit(IfElseStatement& s) override {
        text += "if (";
        s.condition->Accept(*this);
        text += ") ";
        s.body->Accept(*this);
        if (s.else_branch) {
            text.Truncate(text.LastIndexOf('}') + 1);
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

    void Visit(Module& m) override {
        for (auto& ss : m.statements) {
            ss->Accept(*this);
        }
    }
};

}
