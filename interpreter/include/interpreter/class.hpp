#pragma once

#include "common/values/value.hpp"
#include "common/heap/heap.hpp"
#include "common/objects/string.hpp"
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

    bool IsPrototypeOf(const Handle<Class>& other) const {
        if (other->prototype_chain.empty())
            return false;

        return std::any_of(other->prototype_chain.begin(), other->prototype_chain.end(),
                           [&](const Handle<HeapValue> v) {
                               Handle<Class> cl = v.As<Class>();
                               return *name == *cl->name;
                           });
    }

    bool IsDirectPrototypeOf(const Handle<Class>& other) const {
        return !other->prototype_chain.empty() && *name == *other->prototype_chain[0].As<Class>()->name;
    }

    size_t AddField(Handle<String> field_name) {
        size_t pos = field_index_offset + field_name_to_index.size() - 1;
        field_name_to_index[field_name] = pos;
        return pos;
    }

    size_t GetFieldIndex(Handle<String> field_name) const {
        return field_name_to_index.at(field_name);
    }

    size_t GetFieldIndexOffset() const {
        return field_index_offset;
    }

    size_t GetSelfFieldCount() const {
        return field_name_to_index.size();
    }

    size_t GetFieldCount() const {
        return field_index_offset + field_name_to_index.size();
    }

    size_t AddMethod(Handle<String> method_name, Handle<Closure> method_target) {
        methods.push_back(method_target);
        size_t pos = method_index_offset + methods.size() - 1;
        method_name_to_index[method_name] = pos;
        return pos;
    }

    Handle<Closure>& GetMethod(size_t index) {
        if (index >= method_index_offset)
            return methods.at(index);

        for (auto& proto : prototype_chain) {
            return proto.As<Class>()->GetMethod(index);
        }
        throw std::runtime_error("Cannot find method with specified index");
    }

    size_t GetMethodIndex(Handle<String> method_name) const {
        return method_name_to_index.at(method_name);
    }

    size_t GetMethodIndexOffset() const {
        return method_index_offset;
    }

    Handle<Closure>& GetMethod(Handle<String> method_name) {
        return methods[GetMethodIndex(method_name)];
    }

    size_t GetSelfMethodCount() const {
        return method_index_offset + method_name_to_index.size();
    }

    size_t GetMethodCount() const {
        return method_name_to_index.size();
    }

    Handle<Closure>& GetOverload(Overloads target) {
        NLANG_ASSERT(target != Overloads::OVERLOAD_COUNT);

        return overloads_mapping[(size_t) target];
    }

    std::vector<Handle<HeapValue>>& GetPrototypeChain() {
        return prototype_chain;
    }

    bool operator==(const Handle<Class>& other) const {
        return *name == *other->name;
    }

    bool operator!=(const Handle<Class>& other) const {
        return !(*this == other);
    }

    void ForEachReference(std::function<void(Handle<Value>)> handler) override {
        handler(name);
        std::for_each(prototype_chain.begin(), prototype_chain.end(), handler);
        std::for_each(methods.begin(), methods.end(), handler);
        std::for_each(overloads_mapping.begin(), overloads_mapping.end(), handler);

        for (auto kv : field_name_to_index) {
            handler(kv.first);
        }

        for (auto kv : method_name_to_index) {
            handler(kv.first);
        }
    }

    Class() = delete;
    Class(const Class&) = delete;
    Class(Class&&) = delete;

    Class& operator=(const Class&) = delete;
    Class& operator=(const Class&&) = delete;

    virtual ~Class() override = default;

private:
    explicit Class(Handle<String> class_name) : name(class_name), field_index_offset(0),
                                                method_index_offset(0) {}

    Class(Handle<String> class_name, Handle<Class> class_prototype) : name(class_name),
                                                                      field_index_offset(0), method_index_offset(0) {
        ResolvePrototypingFrom(class_prototype);
    }

    void ResolvePrototypingFrom(Handle<Class> class_prototype) {
        prototype_chain.emplace_back(class_prototype);
        auto parent_chain = class_prototype->GetPrototypeChain();
        prototype_chain.insert(prototype_chain.end(), parent_chain.begin(), parent_chain.end());
        field_index_offset += class_prototype->GetFieldCount();
        method_index_offset += class_prototype->GetMethodCount();
        prototype_chain.shrink_to_fit();
    }

    Handle<String> name;
    std::vector<Handle<HeapValue>> prototype_chain;
    size_t field_index_offset;
    size_t method_index_offset;
    std::vector<Handle<Closure>> methods;
    std::unordered_map<Handle<String>, size_t> field_name_to_index;
    std::unordered_map<Handle<String>, size_t> method_name_to_index;
    std::array<Handle<Closure>, (size_t) Overloads::OVERLOAD_COUNT> overloads_mapping;
};

}
