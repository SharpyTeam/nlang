#pragma once

#include "value.hpp"
#include "heap.hpp"
#include "string.hpp"
#include "function.hpp"

#include <unordered_map>
#include <array>

namespace nlang {

#define OVERLOADS_LIST                               \
T(ADD, add, "add")                                   \
T(SUB, sub, "sub")                                   \
T(MUL, mul, "mul")                                   \
T(DIV, div, "div")                                   \
T(EQUALS, equals, "eq")                              \
T(NOT_EQUALS, not_equals, "neq")                     \
T(ASSIGN, assign, "assign")                          \
T(GREATER, greater, "greater")                       \
T(GREATER_EQUALS, greater_equals, "greaterEq")       \
T(LESS, less, "less")                                \
T(LESS_EQUALS, less_equals, "lessEq")                \
T(ASSIGN_ADD, assign_add, "assignAdd")               \
T(ASSIGN_SUB, assign_sub, "assignSub")               \
T(ASSIGN_MUL, assign_mul, "assignMul")               \
T(ASSIGN_DIV, assign_div, "assignDiv")               \
T(SUBST, subst, "subst")                             \
T(INC, inc, "inc")                                   \
T(DEC, dec, "dec")                                   \


class Class : public HeapValue {
public:
    enum class Overloads : uint8_t {
#define T(name, f_name, real_name) name,
        OVERLOADS_LIST
#undef T
        OVERLOAD_COUNT
    };

    static Handle<Class> New(Heap& heap, Handle<String> class_name) {
        return heap.Store(new Class(class_name)).As<Class>();
    }

    Handle<String> GetName() const {
        return name;
    }

    size_t AddField(Handle<String> field_name) {
        size_t pos = field_name_to_index.size() - 1;
        field_name_to_index[field_name] = pos;
        return pos;
    }

    size_t GetFieldIndex(Handle<String> field_name) const {
        return field_name_to_index.at(field_name);
    }

    size_t GetFieldCount() const {
        return field_name_to_index.size();
    }

    size_t AddMethod(Handle<String> method_name, Handle<InterpretedFunction> method_target) {
        methods.push_back(method_target);
        size_t pos = methods.size() - 1;
        method_name_to_index[method_name] = pos;
        return pos;
    }

    Handle<InterpretedFunction>& GetMethod(size_t index) {
        return methods[index];
    }

    size_t GetMethodIndex(Handle<String> method_name) const {
        return method_name_to_index.at(method_name);
    }

    size_t GetMethodCount() const {
        return method_name_to_index.size();
    }

    Handle<InterpretedFunction>& GetOverload(Overloads target) {
        NLANG_ASSERT(target != Overloads::OVERLOAD_COUNT);

        return overloads_mapping[(size_t) target];
    }

    // TODO: == by full name
    // TODO: prototyping interfaces

    static constexpr Value::Type TYPE = Value::Type::CLASS;

    Class() = delete;
    Class(const Class&) = delete;
    Class(Class&&) = delete;

    Class& operator=(const Class&) = delete;
    Class& operator=(const Class&&) = delete;

private:
    explicit Class(Handle<String> class_name) : HeapValue(Type::CLASS), name(class_name) {}
    Class(Handle<String> class_name, Handle<Class> class_prototype) : HeapValue(Type::CLASS), name(class_name), prototype(class_prototype) {
        // TODO: deriving handling
    }

    Handle<String> name;
    Handle<HeapValue> prototype;
    std::vector<Handle<InterpretedFunction>> methods;
    std::unordered_map<Handle<String>, size_t> field_name_to_index;
    std::unordered_map<Handle<String>, size_t> method_name_to_index;
    std::array<Handle<InterpretedFunction>, (size_t) Overloads::OVERLOAD_COUNT> overloads_mapping;
};

}
