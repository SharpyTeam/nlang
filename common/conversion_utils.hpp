//
// Created by ilya on 26.11.2019.
//

#ifndef NLANG_CONVERSION_UTILS_HPP
#define NLANG_CONVERSION_UTILS_HPP

#include <cstdint>
#include <cstdlib>
#include <cstring>

inline uint8_t *DoubleToBytes(double d) {
    const size_t bytes_in_double = sizeof(double) / sizeof(uint8_t);
    auto *dst = new uint8_t[bytes_in_double];
    std::memcpy(dst, &d, bytes_in_double);
    return dst;
}
inline uint8_t *LongLongToBytes(long long d) {
    const size_t bytes_in_long_long = sizeof(long long) / sizeof(uint8_t);
    auto *dst = new uint8_t[bytes_in_long_long];
    auto *src = reinterpret_cast<uint8_t *>(&d);
    for (size_t i = 0; i < bytes_in_long_long; ++i) {
        dst[i] = src[i];
    }
    return dst;
}

#endif //NLANG_CONVERSION_UTILS_HPP
