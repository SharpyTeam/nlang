#pragma once

#include <common/ast/ast.hpp>
#include <compiler/scope.hpp>

#include <interpreter/bytecode_function.hpp>

#include <utils/shared_ptr.hpp>
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
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNull>();
    }

    void Visit(ast::BoolLiteral& literal) override {
        if (literal.flag) {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadTrue>();
        } else {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadFalse>();
        }
    }

    void Visit(ast::NumberLiteral& literal) override {
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNumber>(literal.number);
    }

    void Visit(ast::StringLiteral& literal) override {
        auto index = GetContext()->GetBytecodeGenerator()->StoreConstant(String::New(heap, literal.string->GetRawString()));
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadConstant>(index);
    }

    void Visit(ast::IdentifierLiteral& literal) override {
        auto location = GetContext()->GetLocation(literal.identifier);
        if (location.storage_type == Scope::StorageType::Register) {
            if (!GetContext()->GetRegistersShape()->IsDeclared(literal.identifier)) {
                throw;
            }
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(location.register_index);
        } else if (location.storage_type == Scope::StorageType::Context) {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadContext>({ location.context_descriptor.index, location.context_descriptor.depth });
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
        auto location = GetContext()->GetLocation(part.name->identifier);
        if (location.storage_type == Scope::StorageType::Context) {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(-(uint32_t)part.index - 1);
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>({ location.context_descriptor.index, location.context_descriptor.depth });
        } else if (location.storage_type == Scope::StorageType::Register) {
            GetContext()->GetRegistersShape()->Declare(part.name->identifier);
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
        expression.left->Accept(*this);
        auto left = GetContext()->GetRegistersShape()->LockRegisters(1);
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(left.index);
        expression.right->Accept(*this);
        auto right = GetContext()->GetRegistersShape()->LockRegisters(1);
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(right.index);
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(left.index);
        GetContext()->GetRegistersShape()->ReleaseRegisters(left);
        switch (expression.op.token) {
            case Token::ADD: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Add>(right.index);
                break;
            }
            case Token::SUB: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Sub>(right.index);
                break;
            }
            case Token::MUL: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Mul>(right.index);
                break;
            }
            case Token::DIV: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Div>(right.index);
                break;
            }
            case Token::EQUALS: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckEqual>(right.index);
                break;
            }
            case Token::NOT_EQUALS: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckNotEqual>(right.index);
                break;
            }
            case Token::GREATER: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckGreater>(right.index);
                break;
            }
            case Token::GREATER_EQUALS: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckGreaterOrEqual>(right.index);
                break;
            }
            case Token::LESS: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckLess>(right.index);
                break;
            }
            case Token::LESS_EQUALS: {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CheckLessOrEqual>(right.index);
                break;
            }
            default:
                throw;
        }
        GetContext()->GetRegistersShape()->ReleaseRegisters(right);
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
        auto f = GetContext()->GetRegistersShape()->LockRegisters(1);
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(f.index);
        auto args = GetContext()->GetRegistersShape()->LockRegisters(expression.arguments.size());
        for (size_t i = 0; i < expression.arguments.size(); ++i) {
            expression.arguments[i]->Accept(*this);
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(args.index + i);
        }
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadRegister>(f.index);
        GetContext()->GetRegistersShape()->ReleaseRegisters(f);
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Call>({ args.index, args.count });
        GetContext()->GetRegistersShape()->ReleaseRegisters(args);
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
        PushContext(statement);

        for (auto& arg : statement.arguments) {
            arg->Accept(*this);
        }

        if (statement.type_hint) {
            statement.type_hint->Accept(*this);
        }

        statement.body->Accept(*this);

        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNull>();
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Return>();
        GetContext()->GetBytecodeGenerator()->SetArgumentsCount(statement.arguments.size());
        GetContext()->GetBytecodeGenerator()->SetRegistersCount(GetContext()->GetRegistersShape()->GetRegistersCount());
        auto context = GetContext();
        PopContext();

        auto f = BytecodeFunction::New(heap, context->GetBytecodeGenerator()->Flush());
        auto f_index = GetContext()->GetBytecodeGenerator()->StoreConstant(f);

        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadConstant>(f_index);
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::CreateClosure>();

        auto location = GetContext()->GetLocation(statement.name->identifier);
        if (location.storage_type == Scope::StorageType::Register) {
            GetContext()->GetRegistersShape()->Declare(statement.name->identifier);
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(location.register_index);
        } else if (location.storage_type == Scope::StorageType::Context) {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::DeclareContext>({ location.context_descriptor.index, location.context_descriptor.depth });
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>({ location.context_descriptor.index, location.context_descriptor.depth });
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

        auto location = GetContext()->GetLocation(statement.name->identifier);
        if (location.storage_type == Scope::StorageType::Register) {
            GetContext()->GetRegistersShape()->Declare(statement.name->identifier);
            if (statement.default_value) {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreRegister>(location.register_index);
            }
        } else if (location.storage_type == Scope::StorageType::Context) {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::DeclareContext>({ location.context_descriptor.index, location.context_descriptor.depth });
            if (statement.default_value) {
                GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::StoreContext>({ location.context_descriptor.index, location.context_descriptor.depth });
            }
        } else {
            throw;
        }
    }

    void Visit(ast::ExpressionStatement& statement) override {
        statement.expression->Accept(*this);
    }

    void Visit(ast::BlockStatement& statement) override {
        PushWeakContext(statement);
        for (auto& s : statement.statements) {
            s->Accept(*this);
        }
        PopContext();
    }

    void Visit(ast::IfElseStatement& statement) override {
        statement.condition->Accept(*this);

        auto if_false_label = GetContext()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::JumpIfFalse>(0);

        statement.body->Accept(*this);

        if (statement.else_branch) {
            auto else_skipper_label = GetContext()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::Jump>(0);
            GetContext()->GetBytecodeGenerator()->UpdateJumpToHere(if_false_label);

            statement.else_branch->Accept(*this);

            GetContext()->GetBytecodeGenerator()->UpdateJumpToHere(else_skipper_label);
        } else {
            GetContext()->GetBytecodeGenerator()->UpdateJumpToHere(if_false_label);
        }
    }

    void Visit(ast::WhileStatement& statement) override {
        bytecode::Label first_while_instruction = GetContext()->GetBytecodeGenerator()->GetLabel();
        statement.condition->Accept(*this);
        auto if_false_label = GetContext()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::JumpIfFalse>(0);
        statement.body->Accept(*this);
        GetContext()->GetBytecodeGenerator()->EmitJump<bytecode::Opcode::Jump>(first_while_instruction);
        GetContext()->GetBytecodeGenerator()->UpdateJumpToHere(if_false_label);
    }

    void Visit(ast::ReturnStatement& statement) override {
        if (statement.expression) {
            statement.expression->Accept(*this);
        } else {
            GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::LoadNull>();
        }
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Return>();
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
        PushContext(module);
        for (auto& s : module.statements) {
            s->Accept(*this);
        }

        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::Return>();
        GetContext()->GetBytecodeGenerator()->SetArgumentsCount(0);
        GetContext()->GetBytecodeGenerator()->SetRegistersCount(GetContext()->GetRegistersShape()->GetRegistersCount());
        auto context = GetContext();
        PopContext();

        result = BytecodeFunction::New(heap, context->GetBytecodeGenerator()->Flush());
    }

private:
    template<bool weak>
    NLANG_FORCE_INLINE void PushContextImpl(ast::INode& node) {
        NLANG_ASSERT(node.meta);
        scope_stack.push_front(SharedCast<Scope>(node.meta));
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::PushContext>(GetContext()->GetCount(Scope::StorageType::Context));
    }

    void PushContext(ast::INode& node) {
        PushContextImpl<false>(node);
    }

    void PushWeakContext(ast::INode& node) {
        PushContextImpl<true>(node);
    }

    void PopContext() {
        GetContext()->GetBytecodeGenerator()->EmitInstruction<bytecode::Opcode::PopContext>();
        scope_stack.pop_front();
    }

    SharedPtr<Scope> GetContext(size_t depth = 0) {
        return scope_stack[depth];
    }

    Handle<Function> result;
    Heap* heap;
    std::deque<SharedPtr<Scope>> scope_stack;
};

}