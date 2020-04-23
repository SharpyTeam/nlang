#pragma once

#include "interpreter.hpp"
#include <interpreter/thread.hpp>
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
                    if (thread->acc.Is<Number>() && thread->sp->registers[thread->ip->reg].Is<Number>())
                        thread->acc = Number::New(thread->acc.As<Number>()->Value() + thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    else if (thread->acc.Is<String>() && thread->sp->registers[thread->ip->reg].Is<String>())
                        thread->acc = String::New(thread->heap, *thread->acc.As<String>(), *thread->sp->registers[thread->ip->reg].As<String>());
                    else if (thread->acc.Is<String>()) {
                        thread->acc = String::New(thread->heap, *thread->acc.As<String>(), std::to_string(thread->sp->registers[thread->ip->reg].As<Number>()->Value()));
                    }
                    else {
                        thread->acc = String::New(thread->heap, std::to_string(thread->sp->registers[thread->ip->reg].As<Number>()->Value()), *thread->acc.As<String>());
                    }
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
                case Opcode::CheckEqual: {
                    thread->acc = Bool::New(almost_equal(thread->acc.As<Number>()->Value(), thread->sp->registers[thread->ip->reg].As<Number>()->Value(), 20));
                    break;
                }
                case Opcode::CheckNotEqual: {
                    thread->acc = Bool::New(!almost_equal(thread->acc.As<Number>()->Value(), thread->sp->registers[thread->ip->reg].As<Number>()->Value(), 20));
                    break;
                }
                case Opcode::CheckGreater: {
                    thread->acc = Bool::New(thread->acc.As<Number>()->Value() > thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    break;
                }
                case Opcode::CheckGreaterOrEqual: {
                    thread->acc = Bool::New(
                            almost_equal(thread->acc.As<Number>()->Value(), thread->sp->registers[thread->ip->reg].As<Number>()->Value(), 20) ||
                                  thread->acc.As<Number>()->Value() > thread->sp->registers[thread->ip->reg].As<Number>()->Value()
                            );
                    break;
                }
                case Opcode::CheckLess: {
                    thread->acc = Bool::New(thread->acc.As<Number>()->Value() < thread->sp->registers[thread->ip->reg].As<Number>()->Value());
                    break;
                }
                case Opcode::CheckLessOrEqual: {
                    thread->acc = Bool::New(
                            almost_equal(thread->acc.As<Number>()->Value(), thread->sp->registers[thread->ip->reg].As<Number>()->Value(), 20) ||
                            thread->acc.As<Number>()->Value() < thread->sp->registers[thread->ip->reg].As<Number>()->Value()
                    );
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
                    if (thread->acc.Is<Number>() && !almost_equal(thread->acc.As<Number>()->Value(), 0.0, 20)) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    if (thread->acc.Is<Bool>() && thread->acc.As<Bool>()->Value()) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    if (thread->acc.Is<String>() && thread->acc.As<String>()->GetLength() > 0) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    break;
                }
                case Opcode::JumpIfFalse: {
                    if (thread->acc.Is<Number>() && almost_equal(thread->acc.As<Number>()->Value(), 0.0, 20)) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    if (thread->acc.Is<Bool>() && !thread->acc.As<Bool>()->Value()) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
                    if (thread->acc.Is<String>() && thread->acc.As<String>()->GetLength() == 0) {
                        thread->ip += thread->ip->offset;
                        continue;
                    }
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