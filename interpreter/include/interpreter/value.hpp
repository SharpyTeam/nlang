#pragma once

#include <utils/containers/nan_boxed_primitive.hpp>
#include <utils/macro.hpp>
#include <utils/traits.hpp>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <functional>


namespace nlang {
/**
 * Base class for all objects and primitives
 */
class Value {
public:
    Value(const Value&) = delete;
    Value(Value&&) = delete;
    Value& operator=(const Value&) = delete;
    Value& operator=(Value&&) = delete;

protected:
    Value() {}
};

/**
 * Represents an immediate value, which is stored directly in 8-byte pointer
 */
class StackValue : public NanBoxedPrimitive, public Value {};

template<typename T>
class Handle;

/**
 * Represents a value that is stored in heap
 */
class HeapValue : public Value {
public:
    virtual void ForEachReference(std::function<void(Handle<Value>)> handler) = 0;
    virtual ~HeapValue() = default;
};



}
