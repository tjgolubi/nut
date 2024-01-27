#ifndef TO_H
#define TO_H
#pragma once

#include <gsl/gsl>

#include <array>
#include <string>
#include <string_view>
#include <ranges>
#include <charconv>
#include <system_error>
#include <type_traits>
#include <cctype>
#include <cstring>

template<typename T, typename U>
requires (std::is_same_v<T, std::string> && std::is_arithmetic_v<U>)
constexpr auto To(const U& x) -> T {
  std::array<char, 32> buf;
  auto result = std::to_chars(buf.begin(), buf.end(), x);
  if (result.ec != std::errc{})
    throw std::system_error{std::make_error_code(result.ec)};
  return std::string{buf.data(), result.ptr};
}

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
