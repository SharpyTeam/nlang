#pragma once

#include "value.hpp"

namespace nlang {

class Object : public HeapValue {
public:
    Object() : HeapValue(Type::OBJECT) {}

    static constexpr Value::Type TYPE = Value::Type::OBJECT;
};

}
