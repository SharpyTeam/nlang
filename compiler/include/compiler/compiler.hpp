#pragma once

#include <common/ast.hpp>
#include <compiler/scope.hpp>

#include <interpreter/bytecode_function.hpp>

#include <utils/pointers/shared_ptr.hpp>
#include <utils/macro.hpp>

#include <deque>

namespace nlang {

class Compiler : public ast::IASTVisitor {
public:
    Handle<Function> Compile(Heap* heap_, ast::Module& module) {
        heap = heap_;
        module.Accept(*this);
        NLANG_ASSERT(scope_stack.empty());
        return result;
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
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNull>();
    }

    void Visit(ast::BoolLiteral& literal) override {
        if (literal.flag) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadTrue>();
        } else {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadFalse>();
        }
    }

    void Visit(ast::NumberLiteral& literal) override {
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNumber>(literal.number);
    }

    void Visit(ast::StringLiteral& literal) override {
        auto index = GetScope()->GetBytecodeGenerator()->StoreConstant(String::New(heap, literal.string));
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadConstant>(index);
    }

    void Visit(ast::IdentifierLiteral& literal) override {
        auto location = GetScope()->GetLocation(literal.identifier);
        if (location.storage_type == Scope::StorageType::Register) {
            if (!GetScope()->GetRegistersShape()->IsDeclared(literal.identifier)) {
                throw;
            }
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(location.reg);
        } else if (location.storage_type == Scope::StorageType::Context) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadContext>(location.context_descriptor);
        } else {
            throw;
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
        auto location = GetScope()->GetLocation(part.name->identifier);
        if (location.storage_type == Scope::StorageType::Context) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(-part.index - 1);
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>(location.context_descriptor);
        } else if (location.storage_type == Scope::StorageType::Register) {
            GetScope()->GetRegistersShape()->Declare(part.name->identifier);
        } else {
            throw;
        }
    }

    void Visit(ast::LiteralExpression& expression) override {
        expression.literal->Accept(*this);
    }

    void Visit(ast::ParenthesizedExpression& expression) override {
        expression.expression->Accept(*this);
    }

    void Visit(ast::PrefixExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::PostfixExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::BinaryExpression& expression) override {
        if (expression.op.token == Token::ASSIGN) {
            expression.right->Accept(*this);
            auto literal = static_cast<ast::IdentifierLiteral*>(static_cast<ast::LiteralExpression*>(expression.left.get())->literal.get());
            auto location = GetScope()->GetLocation(literal->identifier);
            if (location.storage_type == Scope::StorageType::Register) {
                if (!GetScope()->GetRegistersShape()->IsDeclared(literal->identifier)) {
                    throw;
                }
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(location.reg);
            } else if (location.storage_type == Scope::StorageType::Context) {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>(location.context_descriptor);
            } else {
                throw;
            }
            return;
        }

        expression.left->Accept(*this);
        auto left = GetScope()->GetRegistersShape()->LockRegisters(1);
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(left.first);
        expression.right->Accept(*this);
        auto right = GetScope()->GetRegistersShape()->LockRegisters(1);
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(right.first);
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(left.first);
        GetScope()->GetRegistersShape()->ReleaseRegisters(left);
        switch (expression.op.token) {
            case Token::ADD: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Add>(right.first);
                break;
            }
            case Token::SUB: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Sub>(right.first);
                break;
            }
            case Token::MUL: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Mul>(right.first);
                break;
            }
            case Token::DIV: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Div>(right.first);
                break;
            }
            case Token::EQUALS: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckEqual>(right.first);
                break;
            }
            case Token::NOT_EQUALS: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckNotEqual>(right.first);
                break;
            }
            case Token::GREATER: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckGreater>(right.first);
                break;
            }
            case Token::GREATER_EQUALS: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckGreaterOrEqual>(right.first);
                break;
            }
            case Token::LESS: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckLess>(right.first);
                break;
            }
            case Token::LESS_EQUALS: {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckLessOrEqual>(right.first);
                break;
            }
            default:
                throw;
        }
        GetScope()->GetRegistersShape()->ReleaseRegisters(right);
    }

    void Visit(ast::OperatorDefinitionExpression& expression) override {
        throw; // not supported
    }

    void Visit(ast::FunctionDefinitionExpression& expression) override {
        throw;
        /*PushScope(expression);

        if (current_pass_type == PassType::Declare) {
            if (expression.name) {
                GetScope()->DeclareLocal(expression.name->identifier);
            }
        }

        for (auto& arg : expression.arguments) {
            arg->Accept(*this);
        }

        if (expression.type_hint) {
            expression.type_hint->Accept(*this);
        }

        expression.body->Accept(*this);

        PopScope();*/
    }

    void Visit(ast::FunctionCallExpression& expression) override {
        expression.expression->Accept(*this);
        auto f = GetScope()->GetRegistersShape()->LockRegisters(1);
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(f.first);
        auto args = GetScope()->GetRegistersShape()->LockRegisters(expression.arguments.size());
        for (int32_t i = 0; i < expression.arguments.size(); ++i) {
            expression.arguments[i]->Accept(*this);
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(args.first + i);
        }
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(f.first);
        GetScope()->GetRegistersShape()->ReleaseRegisters(f);
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Call>(args);
        GetScope()->GetRegistersShape()->ReleaseRegisters(args);
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
        PushScope(statement);

        for (auto& arg : statement.arguments) {
            arg->Accept(*this);
        }

        if (statement.type_hint) {
            statement.type_hint->Accept(*this);
        }

        statement.body->Accept(*this);

        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNull>();
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Return>();
        GetScope()->GetBytecodeGenerator()->SetArgumentsCount(statement.arguments.size());
        GetScope()->GetBytecodeGenerator()->SetRegistersCount(GetScope()->GetRegistersShape()->GetRegistersCount());
        auto context = GetScope();
        PopScope();

        auto f = BytecodeFunction::New(heap, context->GetBytecodeGenerator()->Flush());
        auto f_index = GetScope()->GetBytecodeGenerator()->StoreConstant(f);

        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadConstant>(f_index);
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CreateClosure>();

        auto location = GetScope()->GetLocation(statement.name->identifier);
        if (location.storage_type == Scope::StorageType::Register) {
            GetScope()->GetRegistersShape()->Declare(statement.name->identifier);
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(location.reg);
        } else if (location.storage_type == Scope::StorageType::Context) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::DeclareContext>({ location.context_descriptor.index, location.context_descriptor.depth });
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>({ location.context_descriptor.index, location.context_descriptor.depth });
        } else {
            throw;
        }
    }

    void Visit(ast::VariableDefinitionStatement& statement) override {
        if (statement.type_hint) {
            statement.type_hint->Accept(*this);
        }
        if (statement.default_value) {
            statement.default_value->Accept(*this);
        }

        auto location = GetScope()->GetLocation(statement.name->identifier);
        if (location.storage_type == Scope::StorageType::Register) {
            GetScope()->GetRegistersShape()->Declare(statement.name->identifier);
            if (statement.default_value) {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(location.reg);
            }
        } else if (location.storage_type == Scope::StorageType::Context) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::DeclareContext>({ location.context_descriptor.index, location.context_descriptor.depth });
            if (statement.default_value) {
                GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>({ location.context_descriptor.index, location.context_descriptor.depth });
            }
        } else {
            throw;
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

        auto if_false_label = GetScope()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::JumpIfFalse>(0);

        statement.body->Accept(*this);

        if (statement.else_branch) {
            auto else_skipper_label = GetScope()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::Jump>(0);
            GetScope()->GetBytecodeGenerator()->UpdateJumpToHere(if_false_label);

            statement.else_branch->Accept(*this);

            GetScope()->GetBytecodeGenerator()->UpdateJumpToHere(else_skipper_label);
        } else {
            GetScope()->GetBytecodeGenerator()->UpdateJumpToHere(if_false_label);
        }
    }

    void Visit(ast::WhileStatement& statement) override {
        bytecode::Label first_while_instruction = GetScope()->GetBytecodeGenerator()->GetLabel();
        statement.condition->Accept(*this);
        auto if_false_label = GetScope()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::JumpIfFalse>(0);
        statement.body->Accept(*this);
        GetScope()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::Jump>(first_while_instruction);
        GetScope()->GetBytecodeGenerator()->UpdateJumpToHere(if_false_label);
    }

    void Visit(ast::ReturnStatement& statement) override {
        if (statement.expression) {
            statement.expression->Accept(*this);
        } else {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNull>();
        }
        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Return>();
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

        GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Return>();
        GetScope()->GetBytecodeGenerator()->SetArgumentsCount(0);
        GetScope()->GetBytecodeGenerator()->SetRegistersCount(GetScope()->GetRegistersShape()->GetRegistersCount());
        auto context = GetScope();
        PopScope();

        result = BytecodeFunction::New(heap, context->GetBytecodeGenerator()->Flush());
    }

private:
    template<bool weak>
    NLANG_FORCE_INLINE void PushScopeImpl(ast::INode& node) {
        NLANG_ASSERT(node.meta);
        scope_stack.push_front(StaticPointerCast<Scope>(node.meta));
        if (scope_stack.size() == 1 || GetScope()->GetCount(Scope::StorageType::Context)) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::PushContext>(GetScope()->GetCount(Scope::StorageType::Context));
        }
    }

    void PushScope(ast::INode& node) {
        PushScopeImpl<false>(node);
    }

    void PushWeakScope(ast::INode& node) {
        PushScopeImpl<true>(node);
    }

    void PopScope() {
        if (scope_stack.size() == 1 || GetScope()->GetCount(Scope::StorageType::Context)) {
            GetScope()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::PopContext>();
        }
        scope_stack.pop_front();
    }

    IntrusivePtr<Scope> GetScope(int32_t depth = 0) {
        return scope_stack[depth];
    }

    Handle<Function> result;
    Heap* heap;
    std::deque<IntrusivePtr<Scope>> scope_stack;
};

}