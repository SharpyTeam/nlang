#pragma once

#include <utils/macro.hpp>

#include <cstdint>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <cstring>

namespace nlang {

/**
 * Represents a "fake" nan-boxed primitive.
 * It is used on systems that doesn't allow nan-boxing
 * (for example, on x32 where the sizeof(void*) is 4 bytes).
 */
class FakeNanBoxedPrimitive;


/**
 * Represents a nan-boxed primitive.
 * It can store a pointer to heap or double, int, boolean or null.
 */
class NanBoxedPrimitive {
public:
    static_assert((sizeof(void*) == 8) && std::numeric_limits<double>::is_iec559, "nan boxing is only supported on 64-bit platforms with IEEE754 doubles");



    NLANG_FORCE_INLINE NanBoxedPrimitive() {
        SetPointer(nullptr);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(bool value_) {
        SetBool(value_);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(double value_) {
        SetNumber(value_);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(int32_t value_) {
        SetInt32(value_);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(void* ptr) {
        SetPointer(ptr);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(const NanBoxedPrimitive&) = default;
    NLANG_FORCE_INLINE NanBoxedPrimitive& operator=(const NanBoxedPrimitive&) = default;

    NLANG_FORCE_INLINE NanBoxedPrimitive(const FakeNanBoxedPrimitive&);
    NLANG_FORCE_INLINE NanBoxedPrimitive& operator=(const FakeNanBoxedPrimitive&);

    NLANG_FORCE_INLINE bool IsNull() const {
        return value == null_signature;
    }

    NLANG_FORCE_INLINE bool IsBool() const {
        return (value & ~bool_value_mask) == bool_signature;
    }

    NLANG_FORCE_INLINE bool IsFalse() const {
        return value == bool_false_signature;
    }

    NLANG_FORCE_INLINE bool IsTrue() const {
        return value == bool_true_signature;
    }

    NLANG_FORCE_INLINE bool IsNumber() const {
        return (value & nan_mask) != nan_signature;
    }

    NLANG_FORCE_INLINE bool IsInt32() const {
        return (value & type_mask) == int32_signature;
    }

    NLANG_FORCE_INLINE bool IsPointer() const {
        return (value & type_mask) == pointer_signature;
    }

    NLANG_FORCE_INLINE bool IsNullPointer() const {
        return value == null_pointer_signature;
    }

    NLANG_FORCE_INLINE bool GetBool() const {
        return value == bool_true_signature;
    }

    NLANG_FORCE_INLINE double GetNumber() const {
        double v_;
        std::memcpy(&v_, &value, sizeof(double));
        return v_;
    }

    NLANG_FORCE_INLINE int32_t GetInt32() const {
        int32_t v_;
        std::memcpy(&v_, &value, sizeof(int32_t));
        return v_;
    }

    NLANG_FORCE_INLINE void* GetPointer() const {
        return reinterpret_cast<void*>(SignExtend<47, uint64_t>(value & mask_48_bit));
    }


    NLANG_FORCE_INLINE void SetNull() {
        value = null_signature;
    }

    NLANG_FORCE_INLINE void SetBool(bool value_) {
        value = value_ ? bool_true_signature : bool_false_signature;
    }

    NLANG_FORCE_INLINE void SetNumber(double value_) {
        std::memcpy(&value, &value_, sizeof(uint64_t));
    }

    NLANG_FORCE_INLINE void SetInt32(int32_t value_) {
        value = int32_signature;
        std::memcpy(&value, &value_, sizeof(int32_t));
    }

    NLANG_FORCE_INLINE void SetPointer(void* ptr) {
        value = pointer_signature | (reinterpret_cast<uint64_t>(ptr) & mask_48_bit);
    }


private:
    template<unsigned int bit_num, typename T>
    NLANG_FORCE_INLINE static std::enable_if_t<std::is_integral_v<T>, T> SignExtend(T value) {
        static constexpr T bit_mask = (T)1 << (T)bit_num;
        return value | ((T)0 - (value & bit_mask));
    }

private:
    uint64_t value;

    // bits [50; 49]
    // 00 pointer
    // 01 int32
    // 10 null
    // 11 bool
    static constexpr uint64_t nan_mask =                0x7FFC000000000000;
    static constexpr uint64_t nan_signature =           0x7FFC000000000000;

    static constexpr uint64_t type_mask =               nan_mask | ((uint64_t)0b11 << (uint64_t)48);

    static constexpr uint64_t mask_48_bit =             ((uint64_t)1 << (uint64_t)48) - (uint64_t)1;
    static constexpr uint64_t pointer_signature =       nan_signature | ((uint64_t)0b00 << (uint64_t)48);
    static constexpr uint64_t int32_signature =         nan_signature | ((uint64_t)0b01 << (uint64_t)48);
    static constexpr uint64_t null_signature =          nan_signature | ((uint64_t)0b10 << (uint64_t)48);
    static constexpr uint64_t bool_signature =          nan_signature | ((uint64_t)0b11 << (uint64_t)48);

    static constexpr uint64_t bool_value_mask =         ((uint64_t)0b1 << (uint64_t)0);
    static constexpr uint64_t bool_false_signature =    bool_signature | ((uint64_t)0b0 << (uint64_t)0);
    static constexpr uint64_t bool_true_signature =     bool_signature | ((uint64_t)0b1 << (uint64_t)0);

    static inline uint64_t null_pointer_signature =     pointer_signature | (reinterpret_cast<uint64_t>(nullptr) & mask_48_bit);
};


class FakeNanBoxedPrimitive {
public:
    NLANG_FORCE_INLINE FakeNanBoxedPrimitive() : ptr(nullptr) {}

    NLANG_FORCE_INLINE FakeNanBoxedPrimitive(bool) {
        throw std::runtime_error("not implemented in fake nan boxed primitive");
    }

    NLANG_FORCE_INLINE FakeNanBoxedPrimitive(double) {
        throw std::runtime_error("not implemented in fake nan boxed primitive");
    }

    NLANG_FORCE_INLINE FakeNanBoxedPrimitive(int32_t) {
        throw std::runtime_error("not implemented in fake nan boxed primitive");
    }

    NLANG_FORCE_INLINE FakeNanBoxedPrimitive(void* ptr) : ptr(ptr) {}

    NLANG_FORCE_INLINE FakeNanBoxedPrimitive(const FakeNanBoxedPrimitive&) = default;
    NLANG_FORCE_INLINE FakeNanBoxedPrimitive& operator=(const FakeNanBoxedPrimitive&) = default;

    NLANG_FORCE_INLINE FakeNanBoxedPrimitive(const NanBoxedPrimitive& other) : ptr(other.GetPointer()) {}
    NLANG_FORCE_INLINE FakeNanBoxedPrimitive& operator=(const NanBoxedPrimitive& other) {
        SetPointer(other.GetPointer());
        return *this;
    }


    NLANG_FORCE_INLINE bool IsNull() const {
        return false;
    }

    NLANG_FORCE_INLINE bool IsBool() const {
        return false;
    }

    NLANG_FORCE_INLINE bool IsFalse() const {
        return false;
    }

    NLANG_FORCE_INLINE bool IsTrue() const {
        return false;
    }

    NLANG_FORCE_INLINE bool IsNumber() const {
        return false;
    }

    NLANG_FORCE_INLINE bool IsInt32() const {
        return false;
    }

    NLANG_FORCE_INLINE bool IsPointer() const {
        return true;
    }


    NLANG_FORCE_INLINE bool GetBool() const {
        throw std::runtime_error("not implemented in fake nan boxed primitive");
    }

    NLANG_FORCE_INLINE double GetNumber() const {
        throw std::runtime_error("not implemented in fake nan boxed primitive");
    }

    NLANG_FORCE_INLINE int32_t GetInt32() const {
        throw std::runtime_error("not implemented in fake nan boxed primitive");
    }

    NLANG_FORCE_INLINE void* GetPointer() const {
        return ptr;
    }


    NLANG_FORCE_INLINE void SetNull() {}
    NLANG_FORCE_INLINE void SetBool(bool value_) {}
    NLANG_FORCE_INLINE void SetNumber(double value_) {}
    NLANG_FORCE_INLINE void SetInt32(int32_t value_) {}

    NLANG_FORCE_INLINE void SetPointer(void* ptr_) {
        ptr = ptr_;
    }

private:
    void* ptr;
};

NLANG_FORCE_INLINE NanBoxedPrimitive::NanBoxedPrimitive(const FakeNanBoxedPrimitive& other) {
    SetPointer(other.GetPointer());
}

NLANG_FORCE_INLINE NanBoxedPrimitive& NanBoxedPrimitive::operator=(const FakeNanBoxedPrimitive& other) {
    SetPointer(other.GetPointer());
    return *this;
}


}
