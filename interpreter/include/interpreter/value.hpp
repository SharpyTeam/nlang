#pragma once

#include <utils/nan_boxed_primitive.hpp>
#include <utils/macro.hpp>
#include <utils/traits.hpp>

#include <cstdint>
#include <limits>
#include <type_traits>


namespace nlang {

// Base class for all objects
class Value {
public:
    enum class Type : uintptr_t {
        THE_NULL,
        BOOL,
        NUMBER,
        INT32,
        STRING,
        CLASS,
        OBJECT,
        FUNCTION
    };

    Value(const Value&) = delete;
    Value(Value&&) = delete;
    Value& operator=(const Value&) = delete;
    Value& operator=(Value&&) = delete;

protected:
    Value() {}
};

class StackValue : public NanBoxedPrimitive, public Value {};

class HeapValue : public Value {
public:
    const Type type;

protected:
    HeapValue(Type type) : type(type) {}
};


}
