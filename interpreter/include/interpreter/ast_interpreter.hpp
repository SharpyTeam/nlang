#pragma once

#include <common/ast.hpp>

#include <memory>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace nlang::ast_interpreter {

class Number;
class String;
class Function;

class Object {
public:
    enum class Type {
        THE_NULL,
        THE_TRUE,
        THE_FALSE,
        NUMBER,
        STRING,
        FUNCTION
    };
    const Type type;

    [[nodiscard]] std::string ToString() const;

    bool GetBool() const;

    const Number& AsNumber() const {
        return reinterpret_cast<const Number&>(*this);
    }

    const String& AsString() const {
        return reinterpret_cast<const String&>(*this);
    }

    Function& AsFunction();

    static std::shared_ptr<Object> GetNull() {
        static auto instance = std::shared_ptr<Object>(new Object(Type::THE_NULL));
        return instance;
    }

    static std::shared_ptr<Object> GetFalse() {
        static auto instance = std::shared_ptr<Object>(new Object(Type::THE_FALSE));
        return instance;
    }

    static std::shared_ptr<Object> GetTrue() {
        static auto instance = std::shared_ptr<Object>(new Object(Type::THE_TRUE));
        return instance;
    }

    static std::shared_ptr<Object> GetBool(bool b) {
        return b ? GetTrue() : GetFalse();
    }

    static std::shared_ptr<Object> GetNumber(double number);
    static std::shared_ptr<Object> GetString(const std::string& string);

protected:
    explicit Object(Type type) : type(type) {}
};

class Number : public Object {
public:
    const double number;

    static std::shared_ptr<Number> Get(double number) {
        return std::shared_ptr<Number>(new Number(number));
    }

private:
    explicit Number(double number) : Object(Type::NUMBER), number(number) {}
};

class String : public Object {
public:
    const std::string string;

    static std::shared_ptr<String> Get(const std::string& string) {
        return std::shared_ptr<String>(new String(string));
    }

private:
    explicit String(const std::string& string) : Object(Type::STRING), string(string) {}
};

class Scope {
public:
    const std::shared_ptr<Scope> parent_scope;

    explicit Scope(const std::shared_ptr<Scope>& parent_scope = nullptr) : parent_scope(parent_scope) {}

    std::shared_ptr<Object> GetObject(const std::string& name) const {
        if (auto it = storage.find(name); it != storage.end()) {
            return it->second;
        }
        if (parent_scope) {
            return parent_scope->GetObject(name);
        }
        throw std::runtime_error("Can't find object \"" + name + "\"");
    }

    void DefineObject(const std::string& name, const std::shared_ptr<Object>& object) {
        if (storage.find(name) != storage.end()) {
            throw std::runtime_error("Redefinition of object \"" + name + "\"");
        }
        storage[name] = object;
    }

    void UpdateObject(const std::string& name, const std::shared_ptr<Object>& object) {
        if (auto it = storage.find(name); it != storage.end()) {
            storage[name] = object;
            return;
        }
        if (parent_scope) {
            parent_scope->UpdateObject(name, object);
            return;
        }
        throw std::runtime_error("Object \"" + name + "\" is not defined");
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> storage;
};

class Function : public Object {
public:
    const std::shared_ptr<Scope> scope;

    explicit Function(const std::shared_ptr<Scope>& scope)
        : Object(Object::Type::FUNCTION)
        , scope(scope)
    {

    }

    virtual std::shared_ptr<Object> Apply(const std::vector<std::shared_ptr<Object>>& args) = 0;
};

class InlineFunction : public Function {
public:
    using FunctionType = std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>>&);

    InlineFunction(const std::shared_ptr<Scope>& scope, const std::function<FunctionType>& f)
        : Function(scope)
        , f(f)
    {

    }

    std::shared_ptr<Object> Apply(const std::vector<std::shared_ptr<Object>>& args) override {
        return f(args);
    }

private:
    const std::function<FunctionType> f;
};

std::string Object::ToString() const {
    switch (type) {
        case Object::Type::THE_TRUE:
            return "true";

        case Object::Type::THE_FALSE:
            return "false";

        case Object::Type::THE_NULL:
            return "null";

        case Object::Type::NUMBER: {
            std::stringstream s;
            s << std::setprecision(std::numeric_limits<double>::digits10) << static_cast<const Number *>(this)->number;
            return s.str();
        }

        case Object::Type::STRING:
            return static_cast<const String*>(this)->string;

        case Object::Type::FUNCTION:
            return "[fn]";
    }
    return "[unknown]";
}

std::shared_ptr<Object> Object::GetNumber(double number) {
    return Number::Get(number);
}

std::shared_ptr<Object> Object::GetString(const std::string &string) {
    return String::Get(string);
}

bool Object::GetBool() const {
    switch (type) {
        case Type::THE_NULL:
        case Type::THE_FALSE:
            return false;

        case Type::NUMBER:
            return AsNumber().number != 0.0;

        case Type::STRING:
            return !AsString().string.empty();

        default:
            return true;
    }
}

Function& Object::AsFunction() {
    return static_cast<Function&>(*this);
}

struct ReturnException : public std::exception {
    std::shared_ptr<Object> result;

    ReturnException(std::shared_ptr<Object> result = nullptr) : result(std::move(result)) {}
};

struct BreakException : public std::exception {

};

struct ContinueException : public std::exception {

};

class ASTExecutor : public ASTVisitor {
public:
    std::shared_ptr<Scope> current_scope;
    std::shared_ptr<Object> expression_result;
    std::string identifier_result;

    explicit ASTExecutor(const std::shared_ptr<Scope>& current_scope) : current_scope(current_scope) {}

    void Visit(ASTNode &node) override {}

    void Visit(FileNode &node) override {
        for (auto& s : node.statements) {
            s->Accept(*this);
        }
    }

    void Visit(LiteralExpression &expression) override {
        switch (expression.type) {
            case Token::THE_NULL:
                expression_result = Object::GetNull();
                return;

            case Token::THE_FALSE:
                expression_result = Object::GetFalse();
                return;

            case Token::THE_TRUE:
                expression_result = Object::GetTrue();
                return;

            default:
                throw std::runtime_error("1");
        }
    }

    void Visit(NumberLiteralExpression &expression) override {
        expression_result = Number::Get(expression.number);
    }

    void Visit(StringLiteralExpression &expression) override {
        expression_result = String::Get(expression.string);
    }

    void Visit(IdentifierExpression &expression) override {
        identifier_result = expression.identifier;
        expression_result = current_scope->GetObject(expression.identifier);
    }

    void Visit(BinaryExpression &expression) override {
        identifier_result.clear();
        expression.left->Accept(*this);
        auto l = expression_result;
        std::string identifier = identifier_result;
        expression.right->Accept(*this);
        auto r = expression_result;
        if (expression.op == Token::ADD && (l->type == Object::Type::STRING || r->type == Object::Type::STRING)) {
            expression_result = String::Get(l->ToString() + r->ToString());
            return;
        }

        auto process_binary_arithmetic = [&]() {
            if (l->type != Object::Type::NUMBER || r->type != Object::Type::NUMBER) {
                throw std::runtime_error("2");
            }

            double a = static_cast<Number*>(l.get())->number;
            double b = static_cast<Number*>(r.get())->number;

            switch (expression.op) {
                case Token::ASSIGN_ADD:
                case Token::ADD:
                    expression_result = Number::Get(a + b);
                    return;

                case Token::ASSIGN_SUB:
                case Token::SUB:
                    expression_result = Number::Get(a - b);
                    return;

                case Token::ASSIGN_MUL:
                case Token::MUL:
                    expression_result = Number::Get(a * b);
                    return;

                case Token::ASSIGN_DIV:
                case Token::DIV:
                    expression_result = Number::Get(a / b);
                    return;

                case Token::ASSIGN_REMAINDER:
                case Token::REMAINDER:
                    expression_result = Number::Get(int64_t(a) % int64_t(b));
                    return;

                case Token::LESS:
                    expression_result = Object::GetBool(a < b);
                    return;

                case Token::GREATER:
                    expression_result = Object::GetBool(a > b);
                    return;

                case Token::LESS_EQUALS:
                    expression_result = Object::GetBool(a <= b);
                    return;

                case Token::GREATER_EQUALS:
                    expression_result = Object::GetBool(a >= b);
                    return;

                case Token::EQUALS:
                    expression_result = Object::GetBool(a == b);
                    return;

                case Token::NOT_EQUALS:
                    expression_result = Object::GetBool(a != b);
                    return;

                case Token::AND:
                    expression_result = Object::GetBool(bool(a) && bool(b));
                    return;

                case Token::OR:
                    expression_result = Object::GetBool(bool(a) || bool(b));
                    return;

                default:
                    throw std::runtime_error("not implemented");
            }
        };

        if (expression.op == Token::ASSIGN ||
            expression.op == Token::ASSIGN_ADD ||
            expression.op == Token::ASSIGN_SUB ||
            expression.op == Token::ASSIGN_MUL ||
            expression.op == Token::ASSIGN_DIV ||
            expression.op == Token::ASSIGN_REMAINDER)
        {
            if (identifier.empty()) {
                throw std::runtime_error("cannot assign to this");
            }

            if (expression.op == Token::ASSIGN) {
                current_scope->UpdateObject(identifier, r);
                expression_result = r;
                return;
            }

            process_binary_arithmetic();
            current_scope->UpdateObject(identifier, expression_result);
            return;
        }

        if (expression.op == Token::IDENTIFIER) {
            throw std::runtime_error("function as operator is not supported yet");
        }

        if (l->type == Object::Type::NUMBER && r->type == Object::Type::NUMBER) {
            process_binary_arithmetic();
            return;
        }

        auto a = l->GetBool();
        auto b = r->GetBool();

        switch (expression.op) {
            case Token::EQUALS:
                expression_result = Object::GetBool(a == b);
                return;

            case Token::NOT_EQUALS:
                expression_result = Object::GetBool(a != b);
                return;

            case Token::AND:
                expression_result = Object::GetBool(bool(a) && bool(b));
                return;

            case Token::OR:
                expression_result = Object::GetBool(bool(a) || bool(b));
                return;

            default:
                throw std::runtime_error("not implemented");
        }
    }

    void Visit(UnaryExpression &expression) override {
        throw std::runtime_error("not implemented");
    }

    void Visit(PostfixExpression &expression) override {
        throw std::runtime_error("not implemented");
    }

    void Visit(ExpressionStatement &statement) override {
        statement.expression->Accept(*this);
    }

    void Visit(ReturnStatement &statement) override {
        if (!statement.return_expression) {
            throw ReturnException { Object::GetNull() };
        }
        statement.return_expression->Accept(*this);
        throw ReturnException { expression_result };
    }

    void Visit(BlockStatement &statement) override {
        auto c = current_scope;
        current_scope = std::make_shared<Scope>(c);
        for (auto& s : statement.statements) {
            s->Accept(*this);
        }
        current_scope = c;
    }

    void Visit(VarDefStatement &statement) override {
        statement.expression->Accept(*this);
        current_scope->DefineObject(statement.identifier, expression_result);
    }

    void Visit(FunctionCallExpression &expression) override {
        std::vector<std::shared_ptr<Object>> args;
        for (auto& arg : expression.arguments) {
            arg->Accept(*this);
            args.emplace_back(expression_result);
        }
        expression.expression->Accept(*this);
        if (expression_result->type != Object::Type::FUNCTION) {
            throw std::runtime_error("can't call this");
        }
        expression_result = expression_result->AsFunction().Apply(args);
    }

    void Visit(FunctionDefExpression &expression) override;

    void Visit(IfStatement& is) override {
        for (auto& p : is.if_else_if_statement) {
            p.first->Accept(*this);
            if (expression_result->GetBool()) {
                p.second->Accept(*this);
                return;
            }
        }
        if (is.else_statement) {
            is.else_statement->Accept(*this);
        }
    }

    void Visit(WhileStatement& ws) override {
        while (true) {
            ws.condition->Accept(*this);
            if (!expression_result->GetBool()) {
                break;
            }
            try {
                ws.body->Accept(*this);
            } catch (ContinueException&) {
                continue;
            } catch (BreakException&) {
                break;
            }
        }
    }

    void Visit(BreakStatement& bs) override {
        throw BreakException();
    }

    void Visit(ContinueStatement& cs) override {
        throw ContinueException();
    }
};

class ASTFunction : public Function {
public:
    const std::vector<std::string> args_list;
    const std::shared_ptr<BlockStatement> body;
    ASTExecutor executor;

    ASTFunction(const std::shared_ptr<Scope>& scope, FunctionDefExpression& fn_def)
        : Function(scope)
        , args_list(fn_def.args_list)
        , body(std::dynamic_pointer_cast<BlockStatement>(fn_def.body))
        , executor(scope)
    {

    }

    std::shared_ptr<Object> Apply(const std::vector<std::shared_ptr<Object>>& args) override {
        try {
            auto c = executor.current_scope;
            executor.current_scope = std::make_shared<Scope>(c);
            for (size_t i = 0; i < args_list.size(); ++i) {
                executor.current_scope->DefineObject(args_list[i], i >= args.size() ? Object::GetNull() : args[i]);
            }
            for (auto& s : body->statements) {
                s->Accept(executor);
            }
            executor.current_scope = c;
        } catch (ReturnException& ret) {
            return ret.result;
        }
        return Object::GetNull();
    }
};

void ASTExecutor::Visit(nlang::FunctionDefExpression &expression) {
    expression_result = std::make_shared<ASTFunction>(current_scope, expression);
    if (!expression.name.empty()) {
        current_scope->DefineObject(expression.name, expression_result);
    }
}

class Interpreter {
public:
    Interpreter(const std::shared_ptr<FileNode>& script) : script(script), scope(new Scope) {}

    void Run() {
        InitEnv();
        ASTExecutor e(scope);
        script->Accept(e);
    }

private:
    void InitEnv() {
        scope->DefineObject("print", std::make_shared<InlineFunction>(scope, [](const std::vector<std::shared_ptr<Object>>& args) {
            for (auto& arg : args) {
                std::cout << arg->ToString() << " ";
            }
            std::cout << std::endl;
            return Object::GetNull();
        }));
    }

    const std::shared_ptr<FileNode> script;
    std::shared_ptr<Scope> scope;
};

}
