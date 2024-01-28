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

template<typename T, typename U>
requires requires (U x) { static_cast<T>(x); }
constexpr auto To(U x) -> T { return static_cast<T>(x); }

template<typename T, typename U>
requires (std::is_same_v<T, std::string> && std::is_arithmetic_v<U>)
constexpr auto To(const U& x) -> T {
  std::array<char, 32> buf;
  auto result = std::to_chars(buf.begin(), buf.end(), x);
  if (result.ec != std::errc{})
    throw std::system_error{std::make_error_code(result.ec)};
  return std::string{buf.data(), result.ptr};
}

#ifndef STD_HAS_FROM_CHARS_FLOAT

#include <cmath>

namespace std {

template<typename T>
requires (std::is_floating_point_v<T>)
auto from_chars(const char* first, const char* last, T& x) -> from_chars_result {
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
