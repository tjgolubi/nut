/// @file To.h
/// @brief Lightweight type conversion utilities using C++ constraints and
/// from_chars.
/// @copyright Copyright 2023-2025 Terry Golubiewski, all rights reserved.

#ifndef TO_H
#define TO_H
#pragma once

#include <gsl/gsl>

#include <array>
#include <string>
#include <string_view>
#include <ranges>
#include <charconv>
#include <limits>
#include <system_error>
#include <type_traits>
#include <cctype>
#include <cstring>

/// @brief Performs a static cast between compatible types.
/// @tparam T Target type.
/// @tparam U Source type.
/// @param x Value to convert.
/// @return Converted value of type T.
template<typename T, typename U>
requires requires (U x) { static_cast<T>(x); }
constexpr auto To(U x) noexcept -> T { return static_cast<T>(x); }

/// @brief Converts an arithmetic value to a string using std::to_chars.
/// @tparam T Must be std::string.
/// @tparam U Must be an arithmetic type.
/// @param x Value to convert.
/// @return String representation of the numeric value.
/// @throw std::system_error if conversion fails.
template<typename T, typename U>
requires (std::is_same_v<T, std::string> && std::is_arithmetic_v<U>)
constexpr auto To(const U& x) -> T {
  std::array<char, 32> buf;
  auto result = std::to_chars(buf.begin(), buf.end(), x);
  if (result.ec != std::errc{})
    throw std::system_error{std::make_error_code(result.ec)};
  return std::string{buf.data(), result.ptr};
}

/// @brief Converts a non-arithmetic type to a string using std::to_string.
/// @tparam T Must be std::string.
/// @tparam U A type with a valid std::to_string(U) overload.
/// @param x Value to convert.
/// @return String representation of the value.
template<typename T, typename U>
requires (std::is_same_v<T, std::string> && !std::is_arithmetic_v<U>)
  && requires (U x) { std::to_string(x); }
constexpr auto To(U x) noexcept -> T { return std::to_string(x); }

#ifndef STD_HAS_FROM_CHARS_FLOAT

#include <cmath>

namespace std {

/// @brief Fallback implementation of from_chars for floating-point types.
///
/// Provides compatibility on systems without std::from_chars(float/double).
/// Uses std::strtof, std::strtod, or std::strtold based on T.
///
/// @tparam T Floating-point type.
/// @param first Pointer to beginning of input.
/// @param last Pointer to end of input.
/// @param x Output value.
/// @return from_chars_result with status and parse position.
template<typename T>
requires (std::is_floating_point_v<T>)
auto from_chars(const char* first, const char* last, T& x) noexcept
    -> from_chars_result
{
  char* ptr;
  auto y = T{};
  if constexpr (std::is_same_v<T, float>)
    y = std::strtof(first, &ptr);
  else if (std::is_same_v<T, double>)
    y = std::strtod(first, &ptr);
  else if (std::is_same_v<T, long double>)
    y = std::strtold(first, &ptr);
  if (ptr == first)
    return from_chars_result{ptr, std::errc::invalid_argument};
  if (y == std::numeric_limits<T>::infinity())
    return from_chars_result{ptr, std::errc::result_out_of_range};
  x = y;
  return from_chars_result{ptr, std::errc{}};
} // from_chars floating-point

} // std

#endif

/// @brief Converts a character range to a numeric value using std::from_chars.
///
/// This overload is enabled for contiguous character ranges.
/// Validates that T is an arithmetic type.
///
/// @tparam T Target arithmetic type.
/// @tparam R Character range type.
/// @param r Input range.
/// @return Converted value of type T.
/// @throw std::system_error if conversion fails.
template<typename T, std::ranges::contiguous_range R>
requires (std::is_arithmetic_v<T>) && std::ranges::sized_range<R>
constexpr auto To(const R& r) -> T
requires (std::is_same_v<decltype(r.data()), const char*>)
{
  auto rval = T{};
  auto iter = std::ranges::data(r);
  const auto end = iter + std::ranges::size(r);
  while (iter != end && std::isspace(*iter))
    ++iter;
  auto result = std::from_chars(iter, end, rval);
  if (result.ec != std::errc{})
    throw std::system_error{std::make_error_code(result.ec)};
  if (result.ptr != end)
    throw std::system_error{std::make_error_code(std::errc::invalid_argument)};
  return rval;
}

template<typename T>
requires (std::is_arithmetic_v<T>)
constexpr auto To(gsl::czstring str) -> T
{ return To<T>(std::string_view{str, str + std::strlen(str)}); }

#endif
