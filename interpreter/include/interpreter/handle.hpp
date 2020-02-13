#pragma once

#include <utils/defs.hpp>
#include <utils/nan_boxed_primitive.hpp>
#include <utils/traits.hpp>

#include <type_traits>
#include <cstdint>
#include <stdexcept>

namespace nlang {

class Value;
class StackValue;
class HeapValue;

class Null;
class Bool;
class Number;
class Int32;

template<typename T, typename Enable = void>
class Handle;

template<typename T>
class Handle<T, std::enable_if_t<std::is_base_of_v<Value, T>>> {
public:
    friend class Null;
    friend class Bool;
    friend class Number;
    friend class Int32;

    template<typename U, typename Enable>
    friend class Handle;

    friend class Heap;

    using BackingPrimitive = std::conditional_t<std::is_base_of_v<HeapValue, T>, utils::FakeNanBoxedPrimitive, utils::NanBoxedPrimitive>;

    NLANG_FORCE_INLINE Handle() {}

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
        return Get();
    }

    NLANG_FORCE_INLINE const T* operator->() const {
        return Get();
    }

    NLANG_FORCE_INLINE T& operator*() {
        return *Get();
    }

    NLANG_FORCE_INLINE const T& operator*() const {
        return *Get();
    }

    NLANG_FORCE_INLINE bool IsEmpty() const {
        return value.IsPointer() && value.GetPointer() == nullptr;
    }

    template<typename U>
    NLANG_FORCE_INLINE bool Is() const {
        if constexpr (!std::is_base_of_v<Value, U>) {
            return false;
        } else if constexpr (std::is_same_v<Null, U>) {
            return value.IsNull();
        } else if constexpr (std::is_same_v<Bool, U>) {
            return value.IsBool();
        } else if constexpr (std::is_same_v<Number, U>) {
            return value.IsNumber();
        } else if constexpr (std::is_same_v<Int32, U>) {
            return value.IsInt32();
        } else if constexpr (std::is_base_of_v<HeapValue, U>) {
            if constexpr (std::is_same_v<HeapValue, U>) {
                return value.IsPointer();
            } else {
                if constexpr (!std::is_base_of_v<HeapValue, T>) {
                    if (!value.IsPointer()) return false;
                }
                return GetHeapPointerOfType<U>()->type == U::TYPE;
            }
        } else {
            return false;
        }
    }

    template<typename U>
    NLANG_FORCE_INLINE Handle<U> As() const {
        return Handle<U>(value);
    }

private:
    template<typename U>
    NLANG_FORCE_INLINE U* GetHeapPointerOfType() const {
        return static_cast<U*>(*static_cast<HeapValue**>(value.GetPointer()));
    }

    NLANG_FORCE_INLINE T* Get() const {
        NLANG_ASSERT(Is<T>());

        if constexpr (std::is_same_v<Value, T>) {
            throw std::runtime_error("can't dereference Value");
        } else if constexpr (std::is_base_of_v<HeapValue, T>) {
            return GetHeapPointerOfType<T>();
        } else if constexpr (std::is_base_of_v<StackValue, T>) {
            static_assert(std::is_base_of_v<BackingPrimitive, StackValue>);
            return static_cast<T*>(const_cast<BackingPrimitive*>(&value));
        } else {
            static_assert(dependent_false_v<T>, "unexpected type");
        }
    }

    NLANG_FORCE_INLINE explicit Handle(BackingPrimitive value) : value(value) {}

    BackingPrimitive value;
};

}
