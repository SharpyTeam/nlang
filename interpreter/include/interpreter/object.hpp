#pragma once

#include "value.hpp"
#include "class.hpp"

namespace nlang {

class Object : public HeapValue {
public:
    static Handle<Object> New(Heap& heap, Handle<Class> the_class) {
        return heap.Store(new Object(the_class)).As<Object>();
    }

    Handle<Value>& GetFieldByName(Handle<String> field_name) {
        return GetField(_class->GetFieldIndex(field_name));
    }

    Handle<Value>& GetField(size_t field_index) {
        return fields.at(field_index);
    }

    size_t GetFieldCount() const {
        return _class->GetFieldCount();
    }

    Handle<InterpretedFunction>& GetMethod(Handle<String> method_name) {
        return GetMethod(_class->GetMethodIndex(method_name));
    }

    Handle<InterpretedFunction>& GetMethod(size_t index) {
        return _class->GetMethod(index);
    }

    size_t GetMethodCount() const {
        return _class->GetMethodCount();
    }

    // TODO: checking if the object is instance of specific class

    static constexpr Value::Type TYPE = Value::Type::OBJECT;

    Object() = delete;
    Object(const Object&) = delete;
    Object(Object&&) = delete;

    Object& operator=(const Object&) = delete;
    Object& operator=(const Object&&) = delete;
private:
    explicit Object(Handle<Class> the_class) : HeapValue(Type::OBJECT), _class(the_class) {
        fields.resize(the_class->GetFieldCount());
        fields.assign(the_class->GetFieldCount(), Null::New());
    }

    Handle<Class> _class;
    std::vector<Handle<Value>> fields;
};

}
