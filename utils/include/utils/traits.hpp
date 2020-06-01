/**
 * Contains common type traits
 */

#pragma once

#include <type_traits>
#include <utility>
#include <cmath>

/**
 * Dependent false type
 * Returns false at compile time when compiler reaches it.
 * @tparam T The type
 */
template<typename T> struct dependent_false : std::false_type {};
/**
 * Dependent true type
 * Returns true at compile time when compiler reaches it.
 * @tparam T The type
 */
template<typename T> struct dependent_true : std::true_type {};

/**
 * Dependent false value
 * Returns false (bool) at compile time when compiler reaches it.
 * @tparam T The type
 */
template<typename T> inline constexpr bool dependent_false_v = dependent_false<T>::value;
/**
 * Dependent true value
 * Returns true (bool) at compile time when compiler reaches it.
 * @tparam T The type
 */
template<typename T> inline constexpr bool dependent_true_v = dependent_true<T>::value;

template<class Facet>
struct deletable_facet : Facet {
    template<class ...Args>
    deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() {}
};
