//
// Created by selya on 16.01.2020.
//

#ifndef NLANG_VALUE_HPP
#define NLANG_VALUE_HPP

#if UINTPTR_MAX == UINT64_MAX
#define NLANG_USE_NAN_BOXING
#endif

#include <utils/defs.hpp>
#include <utils/traits.hpp>

#ifdef NLANG_USE_NAN_BOXING
#include <utils/nan_boxed_primitive.hpp>
#endif

#include "heap.hpp"

#include <cstdint>
#include <limits>
#include <type_traits>


namespace nlang {

template<typename T, typename Enable = void>
class Handle;

class HeapNull;
class HeapBool;
class HeapNumber;
class HeapFastInt;

class StackNull;
class StackBool;
class StackNumber;
class StackFastInt;

class String;
class Object;


#ifdef NLANG_USE_NAN_BOXING
using Bool = StackBool;
using Number = StackNumber;
using FastInt = StackFastInt;
using Null = StackNull;
#else
using Bool = HeapBool;
using Number = HeapNumber;
using FastInt = HeapFastInt;
using Null = HeapNull;
#endif

// Base class for all objects
class Value {
public:
    Value(const Value&) = delete;
    Value(Value&&) = delete;
    Value& operator=(const Value&) = delete;
    Value& operator=(Value&&) = delete;

protected:
    Value() {}

    enum class Type : uint8_t {
        THE_NULL,
        BOOL,
        NUMBER,
        FAST_INT,
        STRING,
        OBJECT,
    };
};

class ValueProxy {
    template<typename T, typename Enable>
    friend class Handle;

public:
    using Value =
#ifdef NLANG_USE_NAN_BOXING
    NanBoxedPrimitive
#else
    void*
#endif
    ;

    ValueProxy() : value((void*)nullptr) {}
    explicit ValueProxy(Value value) : value(value) {}

    template<typename T>
    NLANG_FORCE_INLINE bool Is() const;

    template<typename T>
    NLANG_FORCE_INLINE const T& As() const;

    template<typename T>
    NLANG_FORCE_INLINE T& As();

    NLANG_FORCE_INLINE bool IsEmpty() const {
#ifdef NLANG_USE_NAN_BOXING
        return value.IsPointer() && (value.GetPointer() == nullptr);
#else
        return value == nullptr;
#endif
    }

private:
    NLANG_FORCE_INLINE bool IsPointer() const {
#ifdef NLANG_USE_NAN_BOXING
        return value.IsPointer();
#else
        return true;
#endif
    }

    NLANG_FORCE_INLINE void* GetPointer() const {
#ifdef NLANG_USE_NAN_BOXING
        return value.GetPointer();
#else
        return value;
#endif
    }

    Value value;
};


class HeapValue : public Value {
    friend class ValueProxy;

protected:
    HeapValue(Type type) : type(type) {}

    Type type;
};


class HeapNull : public HeapValue {
public:
    static constexpr Type TYPE = Type::THE_NULL;

    NLANG_FORCE_INLINE static Handle<HeapNull> New();

private:
    HeapNull() : HeapValue(Type::THE_NULL) {}
};

class HeapBool : public HeapValue {
public:
    static constexpr Type TYPE = Type::BOOL;

    NLANG_FORCE_INLINE bool Value() const {
        return value;
    }

    NLANG_FORCE_INLINE static Handle<HeapBool> New(bool value);

private:
    HeapBool(bool value) : HeapValue(Type::BOOL), value(value) {}

    bool value;
};

class HeapNumber : public HeapValue {
public:
    static constexpr Type TYPE = Type::NUMBER;

    NLANG_FORCE_INLINE double Value() const {
        return value;
    }

    NLANG_FORCE_INLINE static Handle<HeapNumber> New(double value);

private:
    HeapNumber(double value) : HeapValue(Type::NUMBER), value(value) {}

    double value;
};

class HeapFastInt : public HeapValue {
public:
    static constexpr Type TYPE = Type::FAST_INT;

    NLANG_FORCE_INLINE int32_t Value() const {
        return value;
    }

    NLANG_FORCE_INLINE static Handle<HeapFastInt> New(int32_t value);

private:
    HeapFastInt(int32_t value) : HeapValue(Type::FAST_INT), value(value) {}

    int32_t value;
};


#ifdef NLANG_USE_NAN_BOXING

class StackValue : public NanBoxedPrimitive, public Value {};


class StackNull : public StackValue {
public:
    template<typename T, typename Enable>
    friend class Handle;

    NLANG_FORCE_INLINE static Handle<StackNull> New();

private:
    NLANG_FORCE_INLINE StackNull() {
        SetNull();
    }
};

class StackBool : public StackValue {
public:
    template<typename T, typename Enable>
    friend class Handle;

    NLANG_FORCE_INLINE bool Value() const {
        return GetBool();
    }

    NLANG_FORCE_INLINE static Handle<StackBool> New(bool value);

private:
    NLANG_FORCE_INLINE explicit StackBool(bool value) {
        SetBool(value);
    }
};

class StackNumber : public StackValue {
public:
    template<typename T, typename Enable>
    friend class Handle;

    NLANG_FORCE_INLINE double Value() const {
        return GetNumber();
    }

    NLANG_FORCE_INLINE static Handle<StackNumber> New(double value);

private:
    NLANG_FORCE_INLINE explicit StackNumber(double value) {
        SetNumber(value);
    }
};

class StackFastInt : public StackValue {
public:
    template<typename T, typename Enable>
    friend class Handle;

    NLANG_FORCE_INLINE int32_t Value() {
        return GetFastInt();
    }

    NLANG_FORCE_INLINE static Handle<StackFastInt> New(int32_t value);

private:
    NLANG_FORCE_INLINE explicit StackFastInt(int32_t value) {
        SetFastInt(value);
    }
};

#endif

class Object : private HeapValue {

};

class String : private HeapValue {

};


template<typename T>
class Handle<T, std::enable_if_t<std::is_same_v<Value, T>
#ifdef NLANG_USE_NAN_BOXING
    || std::is_base_of_v<StackValue, T>
#endif
    >> {
    friend class StackNull;
    friend class StackBool;
    friend class StackNumber;
    friend class StackFastInt;

    using Type = std::conditional_t<std::is_same_v<Value, T>, ValueProxy, T>;

public:
    template<typename S, typename Enable>
    friend class Handle;

    NLANG_FORCE_INLINE Handle() {}

    template<typename S>
    NLANG_FORCE_INLINE Handle(const Handle<S>& other) : value(other.value) {
        static_assert(std::is_base_of_v<T, S>);
    }

    template<typename S>
    NLANG_FORCE_INLINE Handle& operator=(const Handle<S>& other) {
        static_assert(std::is_base_of_v<T, S>);
        value = ValueProxy(other.value);
        return *this;
    }

    NLANG_FORCE_INLINE Type* operator->() {
        return &value.As<Type>();
    }

    NLANG_FORCE_INLINE const Type* operator->() const {
        return &value.As<Type>();
    }

    NLANG_FORCE_INLINE Type& operator*() {
        return &value.As<Type>();
    }

    NLANG_FORCE_INLINE const Type& operator*() const {
        return &value.As<Type>();
    }

    NLANG_FORCE_INLINE bool IsEmpty() const {
        return value.IsEmpty();
    }

    template<typename U>
    NLANG_FORCE_INLINE Handle<U> As() const {
        if constexpr (std::is_base_of_v<HeapValue, U>) {
            return Handle<U>(value.GetPointer());
        } else {
            return Handle<U>(value);
        }
    }

private:
    NLANG_FORCE_INLINE explicit Handle(ValueProxy value) : value(value) {}

    ValueProxy value;
};


template<typename T>
class Handle<T, std::enable_if_t<std::is_base_of_v<HeapValue, T>>> {
    friend class HeapNull;
    friend class HeapBool;
    friend class HeapNumber;
    friend class HeapFastInt;

public:
    template<typename S, typename Enable>
    friend class Handle;

    NLANG_FORCE_INLINE Handle() : value(nullptr) {}

    template<typename S>
    NLANG_FORCE_INLINE Handle(const Handle<S>& other) : value(other.value) {
        static_assert(std::is_base_of_v<T, S>);
    }

    template<typename S>
    NLANG_FORCE_INLINE Handle& operator=(const Handle<S>& other) {
        static_assert(std::is_base_of_v<T, S>);
        value = other.value;
        return *this;
    }

    NLANG_FORCE_INLINE T* operator->() {
        return GetActualPointer();
    }

    NLANG_FORCE_INLINE const T* operator->() const {
        return GetActualPointer();
    }

    NLANG_FORCE_INLINE T& operator*() {
        return *GetActualPointer();
    }

    NLANG_FORCE_INLINE const T& operator*() const {
        return *GetActualPointer();
    }

    NLANG_FORCE_INLINE bool IsEmpty() const {
        return value == nullptr;
    }

    template<typename U>
    NLANG_FORCE_INLINE Handle<U> As() const {
        return Handle<U>(value);
    }

private:
    T* GetActualPointer() const {
        return Heap::GetHeapObjectPointer<T>(value);
    }

    NLANG_FORCE_INLINE explicit Handle(void* value) : value(value) {}

    void* value;
};


template<typename T>
bool ValueProxy::Is() const {
#ifdef NLANG_USE_NAN_BOXING
    if constexpr (std::is_same_v<T, StackNull>) {
        return value.IsNull();
    } else if constexpr (std::is_same_v<T, StackBool>) {
        return value.IsBool();
    } else if constexpr (std::is_same_v<T, StackNumber>) {
        return value.IsNumber();
    } else if constexpr (std::is_same_v<T, StackFastInt>) {
        return value.IsFastInt();
    } else
#endif
    if constexpr (std::is_base_of_v<HeapValue, T>) {
        if (!IsPointer()) {
            return false;
        }
        return Heap::GetHeapObjectPointer<HeapValue>(GetPointer())->type == T::TYPE;
    } else {
        static_assert(dependent_false_v<T>, "cant check for this type");
    }
}

template<typename T>
T& ValueProxy::As() {
    return const_cast<T&>(const_cast<const ValueProxy*>(this)->As<T>());
}

template<typename T>
const T& ValueProxy::As() const {
#ifdef NLANG_USE_NAN_BOXING
    if constexpr (std::is_base_of_v<StackValue, T>) {
        return static_cast<const T&>(value);
    } else
#endif
    if constexpr (std::is_base_of_v<HeapValue, T>) {
        return *Heap::GetHeapObjectPointer<T>(GetPointer());
    } else if constexpr (std::is_same_v<ValueProxy, T>) {
        return *this;
    } else {
        static_assert(dependent_false_v<T>, "cant cast to this type");
    }
}


#ifdef NLANG_USE_NAN_BOXING

NLANG_FORCE_INLINE Handle<StackNull> StackNull::New() {
    return Handle<StackNull>(ValueProxy(StackNull()));
}

NLANG_FORCE_INLINE Handle<StackBool> StackBool::New(bool value) {
    return Handle<StackBool>(ValueProxy(StackBool(value)));
}

NLANG_FORCE_INLINE Handle<StackNumber> StackNumber::New(double value) {
    return Handle<StackNumber>(ValueProxy(StackNumber(value)));
}

NLANG_FORCE_INLINE Handle<StackFastInt> StackFastInt::New(int32_t value) {
    return Handle<StackFastInt>(ValueProxy(StackFastInt(value)));
}

#endif


NLANG_FORCE_INLINE Handle<HeapNull> HeapNull::New() {
    return Handle<HeapNull>(Heap::StoreHeapObjectPointer(new HeapNull));
}

NLANG_FORCE_INLINE Handle<HeapBool> HeapBool::New(bool value) {
    return Handle<HeapBool>(Heap::StoreHeapObjectPointer(new HeapBool(value)));
}

NLANG_FORCE_INLINE Handle<HeapNumber> HeapNumber::New(double value) {
    return Handle<HeapNumber>(Heap::StoreHeapObjectPointer(new HeapNumber(value)));
}

NLANG_FORCE_INLINE Handle<HeapFastInt> HeapFastInt::New(int32_t value) {
    return Handle<HeapFastInt>(Heap::StoreHeapObjectPointer(new HeapFastInt(value)));
}


}

#endif //NLANG_VALUE_HPP
