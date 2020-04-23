#pragma once

#include <utils/nan_boxed_primitive.hpp>
#include <utils/macro.hpp>
#include <utils/traits.hpp>

#include <cstdint>
#include <limits>
#include <type_traits>


namespace nlang {

// Base class for all objects and primitives
class Value {
public:
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
    virtual ~HeapValue() = default;
};


}
