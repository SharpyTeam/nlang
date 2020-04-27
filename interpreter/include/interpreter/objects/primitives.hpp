#pragma once

#include <interpreter/handle.hpp>
#include <interpreter/value.hpp>

namespace nlang {


class Null : public StackValue {
public:
    NLANG_FORCE_INLINE static Handle<Null> New();

private:
    NLANG_FORCE_INLINE Null() {
        SetNull();
    }
};


class Bool : public StackValue {
public:
    NLANG_FORCE_INLINE bool Value() const {
        return GetBool();
    }

    NLANG_FORCE_INLINE static Handle<Bool> New(bool value);

private:
    NLANG_FORCE_INLINE explicit Bool(bool value) {
        SetBool(value);
    }
};


class Number : public StackValue {
public:
    NLANG_FORCE_INLINE double Value() const {
        return GetNumber();
    }

    NLANG_FORCE_INLINE static Handle<Number> New(double value);

private:
    NLANG_FORCE_INLINE explicit Number(double value) {
        SetNumber(value);
    }
};


class Int32 : public StackValue {
public:
    NLANG_FORCE_INLINE int32_t Value() {
        return GetInt32();
    }

    NLANG_FORCE_INLINE static Handle<Int32> New(int32_t value);

private:
    NLANG_FORCE_INLINE explicit Int32(int32_t value) {
        SetInt32(value);
    }
};


NLANG_FORCE_INLINE Handle<Null> Null::New() {
    return Handle<Null>(Null());
}

NLANG_FORCE_INLINE Handle<Bool> Bool::New(bool value) {
    return Handle<Bool>(Bool(value));
}

NLANG_FORCE_INLINE Handle<Number> Number::New(double value) {
    return Handle<Number>(Number(value));
}

NLANG_FORCE_INLINE Handle<Int32> Int32::New(int32_t value) {
    return Handle<Int32>(Int32(value));
}


}