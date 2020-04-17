#include <interpreter/interpreter.hpp>
#include <interpreter/function.hpp>

namespace nlang {

void Function::Invoke(Thread* thread, Handle<Context> parent_context, Handle<Function> function, size_t args_count, const Handle<Value>* args) {
    Handle<Context> context = function->context_class->Instantiate(thread->heap, parent_context ? parent_context : (thread->sp ? thread->sp->context : Handle<Context>()));
    thread->PushFrame(context, function);
    function->DoInvoke(thread, args_count, args);
}

Handle<Value> Closure::Call(Thread* thread, size_t args_count, const Handle<Value>* args) {
    Invoke(thread, args_count, args);
    return thread->acc;
}

}