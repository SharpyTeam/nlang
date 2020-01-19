//
// Created by selya on 19.01.2020.
//

#ifndef NLANG_NAN_BOXED_PRIMITIVE_HPP
#define NLANG_NAN_BOXED_PRIMITIVE_HPP

#include <utils/defs.hpp>

#include <cstdint>
#include <type_traits>
#include <limits>

namespace nlang {

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
        SetFastInt(value_);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(void* ptr) {
        SetPointer(ptr);
    }

    NLANG_FORCE_INLINE NanBoxedPrimitive(const NanBoxedPrimitive&) = default;
    NLANG_FORCE_INLINE NanBoxedPrimitive& operator=(const NanBoxedPrimitive&) = default;


    NLANG_FORCE_INLINE bool IsNull() const {
        return (value & type_mask) == null_signature;
    }

    NLANG_FORCE_INLINE bool IsBool() const {
        return (value & bool_mask) == bool_signature;
    }

    NLANG_FORCE_INLINE bool IsFalse() const {
        return (value & type_mask) == bool_false_signature;
    }

    NLANG_FORCE_INLINE bool IsTrue() const {
        return (value & type_mask) == bool_true_signature;
    }

    NLANG_FORCE_INLINE bool IsNumber() const {
        return (value & signaling_nan_mask) != signaling_nan_signature;
    }

    NLANG_FORCE_INLINE bool IsFastInt() const {
        return (value & type_mask) == fast_int_signature;
    }

    NLANG_FORCE_INLINE bool IsPointer() const {
        return (value & type_mask) == pointer_signature;
    }


    NLANG_FORCE_INLINE bool GetBool() const {
        return (value & type_mask) == bool_true_signature;
    }

    NLANG_FORCE_INLINE double GetNumber() const {
        return reinterpret_cast<const double&>(value);
    }

    NLANG_FORCE_INLINE int32_t GetFastInt() const {
        return (uint32_t)(value & 0xFFFFFFFF);
    }

    NLANG_FORCE_INLINE void* GetPointer() const {
        return reinterpret_cast<void*>(SignExtend<47>(value));
    }


    NLANG_FORCE_INLINE void SetNull() {
        value = null_signature;
    }

    NLANG_FORCE_INLINE void SetBool(bool value_) {
        value = value_ ? bool_true_signature : bool_false_signature;
    }

    NLANG_FORCE_INLINE void SetNumber(double value_) {
        value = reinterpret_cast<uint64_t&>(value_);
    }

    NLANG_FORCE_INLINE void SetFastInt(int32_t value_) {
        value = fast_int_signature | reinterpret_cast<uint32_t&>(value_);
    }

    NLANG_FORCE_INLINE void SetPointer(void* ptr) {
        value = pointer_signature | reinterpret_cast<uint64_t&>(ptr);
    }


private:
    template<unsigned int bit_num, typename T>
    NLANG_FORCE_INLINE static std::enable_if_t<std::is_integral_v<T>, T> SignExtend(T value) {
        static constexpr T bit_mask = (T)1 << (T)bit_num;
        return value | ((T)0 - (value & bit_mask));
    }

private:
    uint64_t value;

    // bits [50; 48]
    // 000 pointer
    // 001 fast int
    // 010 null
    // x11 bool
    // 100 reserved
    // 101 reserved
    // 110 reserved
    static constexpr uint64_t signaling_nan_mask =      0x7FF8000000000000;
    static constexpr uint64_t signaling_nan_signature = 0x7FF0000000000000;

    static constexpr uint64_t type_mask =               signaling_nan_mask      | ((uint64_t)0x111 << (uint64_t)48);

    static constexpr uint64_t pointer_signature =       signaling_nan_signature | ((uint64_t)0x000 << (uint64_t)48);
    static constexpr uint64_t null_signature =          signaling_nan_signature | ((uint64_t)0x010 << (uint64_t)48);
    static constexpr uint64_t fast_int_signature =      signaling_nan_signature | ((uint64_t)0x001 << (uint64_t)48);

    static constexpr uint64_t bool_mask =               signaling_nan_mask      | ((uint64_t)0x011 << (uint64_t)48);

    static constexpr uint64_t bool_signature =          signaling_nan_signature | ((uint64_t)0x011 << (uint64_t)48);
    static constexpr uint64_t bool_false_signature =    signaling_nan_signature | ((uint64_t)0x011 << (uint64_t)48);
    static constexpr uint64_t bool_true_signature =     signaling_nan_signature | ((uint64_t)0x111 << (uint64_t)48);
};

}

#endif //NLANG_NAN_BOXED_PRIMITIVE_HPP
