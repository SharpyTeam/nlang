#pragma once

#include "value.hpp"
#include "handle.hpp"
#include "function.hpp"

#include <vector>
#include <cstdint>
#include <thread>

namespace nlang {

class Compiler {

};


class Heap;
class Thread;

class Environment {
public:
    Heap* GetHeap() {
        return &heap;
    }

    Thread* GetCurrentThread() const;

    Environment() {

    }

private:
    Heap heap;
};

class Function;

class Thread {
public:
    Environment* GetEnvironment() const {
        return environment;
    }

private:
    struct StackFrame {
        Handle<Function>    function;
        StackFrame*         caller_frame;
        const uint8_t*      caller_instruction_pointer;
        Handle<Value>*      registers_pointer;
        StackFrame*         next_frame;

        StackFrame(
            Handle<Function> function,
            StackFrame* caller_frame,
            const uint8_t* caller_instruction_pointer,
            const Handle<Value>* args_begin, const Handle<Value>* args_end)
            : function(function)
            , caller_frame(caller_frame)
            , caller_instruction_pointer(caller_instruction_pointer)
            , registers_pointer(GetArgumentsPointer() + (function.IsEmpty() ? 0 : function->GetArgumentsCount()))
            , next_frame(reinterpret_cast<StackFrame*>(registers_pointer + (function.IsEmpty() ? 0 : function->GetRegistersCount())))
        {
            // Copy arguments
            Handle<Value>* dst_ptr = GetArgumentsPointer();
            for (const Handle<Value>* src_ptr = args_begin; src_ptr != args_end; ++src_ptr, ++dst_ptr) {
                *dst_ptr = *src_ptr;
            }
        }

        Handle<Value>* GetArgumentsPointer() const {
            return reinterpret_cast<Handle<Value>*>(const_cast<StackFrame*>(this + 1));
        }
    };

    class ExecutionEnd : public std::exception {};

    // ...
    // StackFrame           <- next_frame
    // ------------------------------------------
    // rN
    // ...
    // r1
    // r0                   <- registers_pointer
    // argN
    // ...
    // arg1
    // arg0                 <- arguments_pointer
    // StackFrame
    // ------------------------------------------
    // ...
    // StackFrame           <- caller_frame

    Environment* environment;
    std::thread thread;
    std::vector<Handle<Value>> call_stack;

    Handle<Value>   accumulator;
    StackFrame*     current_frame;
    Handle<Value>*  registers_pointer;
    Handle<Value>*  arguments_pointer;
    const uint8_t*  instruction_pointer;

public:
    Thread(Environment* environment, Handle<Function> function, Handle<Value>* args_begin, Handle<Value>* args_end)
        : environment(environment)
        , current_frame(nullptr)
        , registers_pointer(nullptr)
        , arguments_pointer(nullptr)
        , instruction_pointer(nullptr)
    {
        thread = std::thread([this](Handle<Function> f, std::vector<Handle<Value>> v) {
            Run(f, v.data(), v.data() + v.size());
        }, function, std::vector<Handle<Value>>(args_begin, args_end));
    }

    void Join() {
        thread.join();
    }

private:
    void PushFrame(Handle<Function> function = Handle<Function>(), const Handle<Value>* args_begin = nullptr, const Handle<Value>* args_end = nullptr) {
        StackFrame* frame = current_frame ? current_frame->next_frame : reinterpret_cast<StackFrame*>(call_stack.data());
        new (frame) StackFrame(function, current_frame, instruction_pointer, args_begin, args_end);
        current_frame = frame;
        registers_pointer = frame->registers_pointer;
        arguments_pointer = frame->GetArgumentsPointer();
        instruction_pointer = (function.IsEmpty() || function->GetFunctionType() == Function::FunctionType::NATIVE) ? nullptr : function.As<InterpretedFunction>()->GetInstructionPointer();
    }

    void PopFrame() {
        if (!current_frame->caller_frame) {
            throw ExecutionEnd();
        }
        instruction_pointer = current_frame->caller_instruction_pointer;
        current_frame = current_frame->caller_frame;
        registers_pointer = current_frame->registers_pointer;
        arguments_pointer = current_frame->GetArgumentsPointer();
    }

    void Call(Handle<Function> function, const Handle<Value>* args_begin, const Handle<Value>* args_end) {
        PushFrame(function, args_begin, args_end);

        if (function->GetFunctionType() == Function::FunctionType::NATIVE) {
            function.As<NativeFunction>()->Apply(current_frame->GetArgumentsPointer(), current_frame->registers_pointer, &accumulator);
            PopFrame();
        }
    }

    void Run(Handle<Function> function, const Handle<Value>* args_begin, const Handle<Value>* args_end) {
        call_stack.reserve(8 * 1024 * 1024);

        Call(function, args_begin, args_end);

        try {
            while (true) {
                // switch on opcode
                switch (*instruction_pointer++) {
                    // call
                    case 1: {
                        uint64_t register_begin = *reinterpret_cast<const uint64_t*>(instruction_pointer);
                        instruction_pointer += sizeof(uint64_t);
                        uint64_t register_end = *reinterpret_cast<const uint64_t*>(instruction_pointer);
                        instruction_pointer += sizeof(uint64_t);
                        Call(accumulator.As<Function>(), registers_pointer + register_begin, registers_pointer + register_end);
                        break;
                    }

                    // return
                    case 2: {
                        PopFrame();
                        break;
                    }

                    // load register to accumulator
                    case 3: {
                        uint64_t register_index = *reinterpret_cast<const uint64_t*>(instruction_pointer);
                        instruction_pointer += sizeof(uint64_t);
                        accumulator = registers_pointer[register_index];
                        break;
                    }

                    // load argument to accumulator
                    case 4: {
                        uint64_t argument_index = *reinterpret_cast<const uint64_t*>(instruction_pointer);
                        instruction_pointer += sizeof(uint64_t);
                        accumulator = arguments_pointer[argument_index];
                        break;
                    }

                    // store accumulator to register
                    case 5: {
                        uint64_t register_index = *reinterpret_cast<const uint64_t*>(instruction_pointer);
                        instruction_pointer += sizeof(uint64_t);
                        registers_pointer[register_index] = accumulator;
                        break;
                    }

                    // store accumulator to argument
                    case 6: {
                        uint64_t argument_index = *reinterpret_cast<const uint64_t*>(instruction_pointer);
                        instruction_pointer += sizeof(uint64_t);
                        arguments_pointer[argument_index] = accumulator;
                        break;
                    }
                }
            }
        } catch (ExecutionEnd&) {

        }
    }
};

}
