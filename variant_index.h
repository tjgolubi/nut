#ifndef VARIANT_INDEX_H
#define VARIANT_INDEX_H
#pragma once

#include <variant>

namespace std {

/// Get a numeric index for particular type included in variant type at compile time.
/// @tparam V Variant type to search for index.
/// @tparam T Type, index of which must be returned.
/// @return Index of type T in variant V; or variant size if V does not include an alternative for T.

template<typename V, typename T, size_t I=0>
constexpr size_t variant_index() {
  if constexpr (I >= std::variant_size_v<V>)
    return std::variant_size_v<V>;
  else if constexpr (std::is_same_v<std::variant_alternative_t<I, V>, T>)
    return I;
  else
    return variant_index<V, T, I+1>();
} // variant_index

} // std

#endif
