#pragma once

#include "interpreter.hpp"

#include <common/bytecode.hpp>

namespace nlang {

class BytecodeExecutor {
public:
    static void Execute(Thread* thread) {
        while (true) {
            switch (thread->ip->opcode) {
                case Opcode::RegToAcc: {
                    thread->acc = thread->sp->registers[thread->ip->operand.register_index];
                    break;
                }
                case Opcode::AccToReg: {
                    thread->sp->registers[thread->ip->operand.register_index] = thread->acc;
                    break;
                }
                case Opcode::Add: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() + thread->sp->registers[thread->ip->operand.register_index].As<Number>()->Value());
                    break;
                }
                case Opcode::Sub: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() - thread->sp->registers[thread->ip->operand.register_index].As<Number>()->Value());
                    break;
                }
                case Opcode::Mul: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() * thread->sp->registers[thread->ip->operand.register_index].As<Number>()->Value());
                    break;
                }
                case Opcode::Div: {
                    thread->acc = Number::New(thread->acc.As<Number>()->Value() / thread->sp->registers[thread->ip->operand.register_index].As<Number>()->Value());
                    break;
                }
                case Opcode::Ret: {
                    thread->PopFrame();
                    if (!thread->sp || !thread->ip) {
                        return;
                    }
                    break;
                }
                case Opcode::Call: {
                    thread->acc.As<Closure>()->Call(thread, thread->ip->operand.registers_range.count, thread->sp->registers + thread->ip->operand.registers_range.first);
                    break;
                }
            }

            ++thread->ip;
        }
    }
};

}