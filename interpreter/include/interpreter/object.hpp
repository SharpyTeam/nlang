#pragma once

#include <interpreter/value.hpp>
#include <interpreter/handle.hpp>

#include <interpreter/class.hpp>

namespace nlang {

/**
 * Represents an instance of a class
 */
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

    Handle<Closure>& GetMethod(Handle<String> method_name) {
        return GetMethod(_class->GetMethodIndex(method_name));
    }

    Handle<Closure>& GetMethod(size_t index) {
        return _class->GetMethod(index);
    }

    size_t GetMethodCount() const {
        return _class->GetMethodCount();
    }

    void ForEachReference(std::function<void(Handle<Value>)> handler) override {
        handler(_class);
        std::for_each(fields.begin(), fields.end(), handler);
    }

    // TODO: checking if the object is instance of specific class

    Object() = delete;
    Object(const Object&) = delete;
    Object(Object&&) = delete;

    Object& operator=(const Object&) = delete;
    Object& operator=(const Object&&) = delete;

    virtual ~Object() override = default;
private:
    explicit Object(Handle<Class> the_class) : _class(the_class) {
        fields.resize(the_class->GetFieldCount());
        fields.assign(the_class->GetFieldCount(), Null::New());
    }

    Handle<Class> _class;
    std::vector<Handle<Value>> fields;
};

}
