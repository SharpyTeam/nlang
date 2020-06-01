#pragma once

#include <common/ast.hpp>

#include <compiler/scope.hpp>

#include <utils/pointers.hpp>
#include <utils/macro.hpp>

#include <deque>

namespace nlang {

/**
 * Semantic analyzer
 * Visits generated AST before compiler and processes declarations and definitions
 */
class SemanticAnalyser : public ast::IASTVisitor {
public:
    void Process(ast::INode& node) {
        current_pass_type = PassType::Declare;
        node.Accept(*this);
        NLANG_ASSERT(scope_stack.empty());
        current_pass_type = PassType::Resolve;
        node.Accept(*this);
        NLANG_ASSERT(scope_stack.empty());
    }

private:
    void Visit(ast::INode& node) override {
        throw;
    }

    void Visit(ast::IExpression& expression) override {
        throw;
    }

    void Visit(ast::IStatement& statement) override {
        throw;
    }

    void Visit(ast::NullLiteral& literal) override {
        // nothing
    }

    void Visit(ast::BoolLiteral& literal) override {
        // nothing
    }

    void Visit(ast::NumberLiteral& literal) override {
        // nothing
    }

    void Visit(ast::StringLiteral& literal) override {
        // nothing
    }

    void Visit(ast::IdentifierLiteral& literal) override {
        if (current_pass_type == PassType::Resolve) {
            GetScope()->Touch(literal.identifier);
        }
    }

    void Visit(ast::TypeHint& hint) override {
        throw; // not supported
    }

    void Visit(ast::DefaultValue& value) override {
        value.value->Accept(*this);
    }

    void Visit(ast::ElseStatementPart& part) override {
        part.body->Accept(*this);
    }

    void Visit(ast::ArgumentDefinitionStatementPart& part) override {
        if (current_pass_type == PassType::Declare) {
            GetScope()->DeclareArgument(part.name->identifier, part.index);
        }
        if (part.type_hint) {
            part.type_hint->Accept(*this);
        }
        if (part.default_value) {
            throw; // not supported
        }
    }

    void Visit(ast::LiteralExpression& expression) override {
        expression.literal->Accept(*this);
    }

    void Visit(ast::ParenthesizedExpression& expression) override {
        expression.expression->Accept(*this);
    }

    void Visit(ast::PrefixExpression& expression) override {
        expression.expression->Accept(*this);
    }

    void Visit(ast::PostfixExpression& expression) override {
        expression.expression->Accept(*this);
    }

    void Visit(ast::BinaryExpression& expression) override {
        expression.left->Accept(*this);
        expression.right->Accept(*this);
    }

    void Visit(ast::OperatorDefinitionExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::FunctionDefinitionExpression& expression) override {
        throw;
        /*PushContext(expression);

        if (current_pass_type == PassType::Declare) {
            if (expression.name) {
                GetContext()->DeclareLocal(expression.name->identifier);
            }
        }

        for (auto& arg : expression.arguments) {
            arg->Accept(*this);
        }

        if (expression.type_hint) {
            expression.type_hint->Accept(*this);
        }

        expression.body->Accept(*this);

        PopContext();*/
    }

    void Visit(ast::FunctionCallExpression& expression) override {
        expression.expression->Accept(*this);
        for (auto& arg : expression.arguments) {
            arg->Accept(*this);
        }
    }

    void Visit(ast::SubscriptExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::MemberAccessExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::ClassDefinitionExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::FunctionDefinitionStatement& statement) override {
        if (current_pass_type == PassType::Declare) {
            GetScope()->DeclareLocal(statement.name->identifier);
        }

        PushScope(statement);

        for (auto& arg : statement.arguments) {
            arg->Accept(*this);
        }

        if (statement.type_hint) {
            statement.type_hint->Accept(*this);
        }

        statement.body->Accept(*this);

        PopScope();
    }

    void Visit(ast::VariableDefinitionStatement& statement) override {
        if (current_pass_type == PassType::Declare) {
            GetScope()->DeclareLocal(statement.name->identifier);
        }
        if (statement.type_hint) {
            statement.type_hint->Accept(*this);
        }
        if (statement.default_value) {
            statement.default_value->Accept(*this);
        }
    }

    void Visit(ast::ExpressionStatement& statement) override {
        statement.expression->Accept(*this);
    }

    void Visit(ast::BlockStatement& statement) override {
        PushWeakScope(statement);
        for (auto& s : statement.statements) {
            s->Accept(*this);
        }
        PopScope();
    }

    void Visit(ast::IfElseStatement& statement) override {
        statement.condition->Accept(*this);
        statement.body->Accept(*this);
        if (statement.else_branch) {
            statement.else_branch->Accept(*this);
        }
    }

    void Visit(ast::WhileStatement& statement) override {
        statement.condition->Accept(*this);
        statement.body->Accept(*this);
    }

    void Visit(ast::ReturnStatement& statement) override {
        if (statement.expression) {
            statement.expression->Accept(*this);
        }
    }

    void Visit(ast::BreakStatement& statement) override {
        if (statement.expression) {
            throw; // not supported
        }
        throw; // not supported
    }

    void Visit(ast::ContinueStatement& statement) override {
        throw; // not supported
    }

    void Visit(ast::Module& module) override {
        NLANG_ASSERT(scope_stack.empty());
        PushScope(module);
        for (auto& s : module.statements) {
            s->Accept(*this);
        }
        PopScope();
    }

private:
    template<bool weak>
    NLANG_FORCE_INLINE void PushScopeImpl(ast::INode& node) {
        if (current_pass_type == PassType::Declare) {
            NLANG_ASSERT(!node.meta);
            scope_stack.push_front(IntrusivePtr<Scope>(new Scope(scope_stack.empty() ? nullptr : scope_stack.front().get(), weak)));
            node.meta = scope_stack[0];
        } else {
            NLANG_ASSERT(node.meta);
            scope_stack.push_front(StaticPointerCast<Scope>(node.meta));
        }
    }

    void PushScope(ast::INode& node) {
        PushScopeImpl<false>(node);
    }

    void PushWeakScope(ast::INode& node) {
        PushScopeImpl<true>(node);
    }

    void PopScope() {
        scope_stack.pop_front();
    }

    IntrusivePtr<Scope> GetScope(int32_t depth = 0) {
        return scope_stack[depth];
    }

    enum class PassType {
        Declare,
        Resolve,
    };

    PassType current_pass_type;
    std::deque<IntrusivePtr<Scope>> scope_stack;
};

}