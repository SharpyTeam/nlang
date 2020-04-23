#pragma once

#include "interpreter.hpp"
#include <interpreter/bytecode_function.hpp>

#include <common/bytecode.hpp>

namespace nlang {

class BytecodeExecutor {
public:
    static void Execute(Thread* thread) {
        using namespace bytecode;
        while (true) {
            switch (thread->ip->opcode) {
                case Opcode::LoadRegister: {
                    thread->acc = thread->sp->registers[thread->ip->reg];
                    break;
                }
                case Opcode::StoreRegister: {
                    thread->sp->registers[thread->ip->reg] = thread->acc;
                    break;
                }
                case Opcode::Add: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() + thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    break;
                }
                case Opcode::Sub: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() - thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    break;
                }
                case Opcode::Mul: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() * thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    break;
                }
                case Opcode::Div: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() / thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    break;
                }
                case Opcode::DeclareContext: {
                    thread->sp->context->Declare({ thread->ip->context_descriptor.index, thread->ip->context_descriptor.depth });
                    break;
                }
                case Opcode::LoadContext: {
                    thread->acc = thread->sp->context->Load({ thread->ip->context_descriptor.index, thread->ip->context_descriptor.depth });
                    break;
                }
                case Opcode::StoreContext: {
                    thread->sp->context->Store({ thread->ip->context_descriptor.index, thread->ip->context_descriptor.depth }, thread->acc);
                    break;
                }
                case Opcode::LoadConstant: {
                    thread->acc = thread->sp->function.As<BytecodeFunction>()->bytecode_chunk.constant_pool[thread->ip->const_index];
                    break;
                }
                case Opcode::Call: {
                    thread->acc.As<Closure>()->Call(thread, thread->ip->reg_range.count, thread->sp->registers + thread->ip->reg_range.first);
                    continue;
                }
                case Opcode::Jump: {
                    thread->ip += thread->ip->offset;
                    continue;
                }
                case Opcode::JumpIfTrue: {
                    if (thread->acc.As<Number>()->Value() != 0.0) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    break;
                }
                case Opcode::JumpIfFalse: {
                    if (thread->acc.As<Number>()->Value() == 0.0) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    break;
                }
                case Opcode::PushContext: {
                    thread->sp->context = Context::New(thread->heap, thread->sp->context, thread->ip->immediate_int32);
                    break;
                }
                case Opcode::LoadNumber: {
                    thread->acc = Number::New(thread->ip->immediate_double);
                    break;
                }
                case Opcode::PopContext: {
                    thread->sp->context = thread->sp->context->GetParent();
                    break;
                }
                case Opcode::CreateClosure: {
                    thread->acc = Closure::New(thread->heap, thread->sp->context, thread->acc.As<Function>());
                    break;
                }
                case Opcode::Return: {
                    thread->PopFrame();
                    if (!thread->sp || !thread->ip) {
                        return;
                    }
                    break;
                }
                case Opcode::LoadNull: {
                    thread->acc = Null::New();
                    break;
                }
                case Opcode::LoadFalse: {
                    thread->acc = Bool::New(false);
                    break;
                }
                case Opcode::LoadTrue: {
                    thread->acc = Bool::New(true);
                    break;
                }
                default:
                    break;
            }

            ++thread->ip;
        }
    }
};

}