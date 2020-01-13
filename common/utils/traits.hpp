//
// Created by ayles on 1/11/20.
//

#ifndef NLANG_TRAITS_HPP
#define NLANG_TRAITS_HPP

#include <type_traits>

template<typename T> struct dependent_false : std::false_type {};
template<typename T> struct dependent_true : std::true_type {};

template<typename T> inline constexpr bool dependent_false_v = dependent_false<T>::value;
template<typename T> inline constexpr bool dependent_true_v = dependent_true<T>::value;

#endif //NLANG_TRAITS_HPP
