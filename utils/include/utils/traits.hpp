#pragma once

#include <type_traits>
#include <utility>
#include <cmath>

template<typename T> struct dependent_false : std::false_type {};
template<typename T> struct dependent_true : std::true_type {};

template<typename T> inline constexpr bool dependent_false_v = dependent_false<T>::value;
template<typename T> inline constexpr bool dependent_true_v = dependent_true<T>::value;

template<class Facet>
struct deletable_facet : Facet {
    template<class ...Args>
    deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() {}
};

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp) {
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
           // unless the result is subnormal
           || std::fabs(x - y) < std::numeric_limits<T>::min();
}