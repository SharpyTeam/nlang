#include <interpreter/function.hpp>
#include <interpreter/bytecode_function.hpp>
#include <interpreter/bytecode_executor.hpp>

namespace nlang {

void Function::Invoke(Thread* thread, Handle<Context> context, Handle<Function> function, int32_t args_count, const Handle<Value>* args) {
    thread->PushFrame(context, function);
    function->DoInvoke(thread, args_count, args);
}

Handle<Value> Closure::Call(Thread* thread, int32_t args_count, const Handle<Value>* args) {
    Invoke(thread, args_count, args);
    return thread->acc;
}

void BytecodeFunction::DoInvoke(Thread* thread, int32_t args_count, const Handle<Value>* args) {
    thread->ip = bytecode_chunk.bytecode.data();
    for (int32_t i = 0; i < args_count; ++i) {
        thread->sp->arguments[i] = args[i];
    }
    if (!thread->sp->prev || !thread->sp->prev->ip) {
        BytecodeExecutor::Execute(thread);
    }
}

}