#include <interpreter/interpreter.hpp>
#include <interpreter/function.hpp>

namespace nlang {

Handle<Context> ContextClass::Instantiate(Heap* heap, Handle<Context> parent_context) {
    Handle<Context> context = heap->Store(new Context).As<Context>();
    context->items.resize(items_count + functions.size());
    for (size_t i = 0; i < functions.size(); ++i) {
        context->items[i] = Closure::New(heap, context, functions[i].As<Function>());
    }
    return context;
}

}