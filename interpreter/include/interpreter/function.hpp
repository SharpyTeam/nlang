#pragma once

#include "value.hpp"
#include "stack_values.hpp"
#include "handle.hpp"

#include <cstdint>
#include <vector>
#include <functional>

namespace nlang {

class Environment;

class Function : public HeapValue {
public:
    enum class FunctionType {
        NATIVE,
        INTERPRETED
    };

    Function(FunctionType type) : HeapValue(Value::Type::FUNCTION), function_type(type) {}

    FunctionType GetFunctionType() const {
        return function_type;
    }

    size_t GetArgumentsCount() const;
    size_t GetRegistersCount() const;

    static constexpr Type TYPE = Type::FUNCTION;

private:
    const FunctionType function_type;
};

class InterpretedFunction : public Function {
public:
    const uint8_t* GetInstructionPointer() const {
        return bytecode.data();
    }

    size_t GetArgumentsCount() const {
        return arguments_count;
    }

    size_t GetRegistersCount() const {
        return registers_count;
    }

    static Handle<InterpretedFunction> New(Environment* environment, const std::vector<uint8_t>& bytecode, size_t arguments_count, size_t registers_count);

private:
    InterpretedFunction(const std::vector<uint8_t>& bytecode, size_t arguments_count, size_t registers_count)
        : Function(Function::FunctionType::INTERPRETED)
        , bytecode(bytecode)
        , arguments_count(arguments_count)
        , registers_count(registers_count)
    {

    }

private:
    std::vector<uint8_t> bytecode;
    size_t arguments_count;
    size_t registers_count;
};

class NativeFunction : public Function {
public:
    class NativeFunctionInfo {
    public:
        NativeFunctionInfo(Handle<Value>* args_start, Handle<Value>* args_end, Handle<Value>* return_to)
            : arguments(args_start, args_end)
            , return_to(return_to)
        {

        }

        size_t GetArgumentsCount() const {
            return arguments.size();
        }

        Handle<Value>& GetArgument(size_t index) {
            return arguments[index];
        }

        void SetReturnValue(Handle<Value> value) {
            *return_to = value;
        }

    private:
        std::vector<Handle<Value>> arguments;
        Handle<Value>* return_to;
    };

    NativeFunction(const std::function<void(NativeFunctionInfo&)>& function, size_t arguments_count)
        : Function(Function::FunctionType::NATIVE)
        , function(function)
        , arguments_count(arguments_count)
    {

    }

    size_t GetArgumentsCount() const {
        return arguments_count;
    }

    size_t GetRegistersCount() const {
        return 0;
    }

    void Apply(Handle<Value>* args_start, Handle<Value>* args_end, Handle<Value>* return_to) {
        NativeFunctionInfo info(args_start, args_end, return_to);
        info.SetReturnValue(Null::New());
        function(info);
    }

    static Handle<NativeFunction> New(Environment* environment, const std::function<void(NativeFunctionInfo&)>& function, size_t arguments_count);

private:
    const std::function<void(NativeFunctionInfo&)> function;
    const size_t arguments_count;
};


inline size_t Function::GetArgumentsCount() const {
    switch (function_type) {
        case FunctionType::NATIVE: return static_cast<const NativeFunction*>(this)->GetArgumentsCount();
        case FunctionType::INTERPRETED: return static_cast<const InterpretedFunction*>(this)->GetArgumentsCount();
    }
    return 0;
}

inline size_t Function::GetRegistersCount() const {
    switch (function_type) {
        case FunctionType::NATIVE: return static_cast<const NativeFunction*>(this)->GetRegistersCount();
        case FunctionType::INTERPRETED: return static_cast<const InterpretedFunction*>(this)->GetRegistersCount();
    }
    return 0;
}

}
