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
