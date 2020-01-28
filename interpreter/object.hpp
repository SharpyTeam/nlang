//
// Created by selya on 25.01.2020.
//

#ifndef NLANG_OBJECT_HPP
#define NLANG_OBJECT_HPP

#include "value.hpp"

namespace nlang {

class Object : public HeapValue {

public:
    static constexpr Value::Type TYPE = Value::Type::OBJECT;
};

}

#endif //NLANG_OBJECT_HPP
