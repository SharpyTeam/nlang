#include <interpreter/function.hpp>

#include <interpreter/interpreter.hpp>

namespace nlang {

void Function::RegisterDeleter(Environment *environment) {
    /*environment->GetHeap()->RegisterDeleterForType(Value::Type::FUNCTION, [](HeapValue* value) {
        switch (static_cast<Function*>(value)->GetFunctionType()) {
            case FunctionType::NATIVE: {
                delete static_cast<NativeFunction*>(value);
                break;
            }
            case FunctionType::INTERPRETED: {
                delete static_cast<InterpretedFunction*>(value);
                break;
            }
        }
    });*/
}

Handle<InterpretedFunction> InterpretedFunction::New(Environment *environment, const std::vector<uint8_t> &bytecode, size_t arguments_count, size_t registers_count) {
    return environment->GetHeap()->Store(new InterpretedFunction(bytecode, arguments_count, registers_count)).As<InterpretedFunction>();
}

Handle<NativeFunction> NativeFunction::New(Environment *environment, const std::function<void (NativeFunctionInfo &)> &function, size_t arguments_count) {
    return environment->GetHeap()->Store(new NativeFunction(function, arguments_count)).As<NativeFunction>();
}

}