#pragma once

#include <ranges>
#include <string_view>
#include <concepts>
#include <iostream>
#include <print>
#include <type_traits>
#include <cstdio>

namespace std {

inline constexpr void println(FILE* file) { println(file, ""); }
inline constexpr void println() { println(stdout); }

} // std

namespace tjg {

namespace detail {

/// Trait to detect whether T can be formatted with std::format("{}",
/// ...) for a given type.
template<typename T, typename CharT = char, typename = void>
struct is_formattable : std::false_type {};

template<typename T, typename CharT>
struct is_formattable<T, CharT,
  std::void_t<
    decltype(
      std::declval<std::formatter<T, CharT>>().format(
        std::declval<const T&>(),
        std::declval<std::basic_format_context<
          std::back_insert_iterator<std::basic_string<CharT>>, CharT>&>())
    )
  >> : std::true_type {};

} // detail

/// Concept to detect whether T can be formatted with std::format("{}", ...)
/// for a given type.
template<typename T, typename CharT = char>
concept Formattable = detail::is_formattable<T, CharT>::value;

/// Concept to detect whether T can be formatted with std::ostream
/// for a given type.
template<typename T>
concept Streamable = requires(std::ostream& os, T x) {
  { os << x } -> std::same_as<std::ostream&>;
};

template<typename T>
concept JustStreamable = Streamable<T> && !Formattable<T>;

namespace detail {

inline constexpr
void print_fmt(std::FILE* file, const auto& x) { std::print("{}", x); }

inline constexpr
void print_fmt(std::FILE* file, std::string_view sep, const auto& x) {
  std::fwrite(sep.data(), sep.size(), 1, file);
  print_fmt(file, x);
}

/// RAI stream/stdio synchronization.
class EnsureSync {
  bool was_synced = false;
public:
  EnsureSync() : was_synced{std::ios::sync_with_stdio(true)} { }
  ~EnsureSync() { (void) std::ios::sync_with_stdio(was_synced); }
}; // EnsureSync

inline constexpr
void print_fmt(std::ostream& os, const auto& x)
{ os << x; }

inline constexpr
void print_fmt(std::ostream& os, std::string_view sep, const auto& x)
{ os << sep << x; }

template<Formattable T>
inline constexpr void print_fmt(const T& x)
{ print_fmt(stdout, x); }

template<Formattable T>
inline constexpr void print_fmt(std::string_view sep, const T& x)
{ print_fmt(stdout, sep, x); }

/// @todo These should synchronize with stdout.
template<JustStreamable T>
inline constexpr void print_fmt(const T& x)
{ print_fmt(std::cout, x); }

template<JustStreamable T>
inline constexpr void print_fmt(std::string_view sep, const T& x)
{ print_fmt(std::cout, sep, x); }

} // detail

/// Print fields from a range, separated by sep, no newline.
template<std::ranges::range Range>
requires Formattable<std::ranges::range_value_t<Range>>
void PrintFields(FILE* file, const std::string_view sep, const Range& fields) {
  auto it = fields.begin();
  const auto end = fields.end();
  if (it == end)
    return;
  detail::print_fmt(file, *it++);
  while (it != end)
    detail::print_fmt(file, sep, *it++);
}

/// Print variadic fields separated by `sep`, no newline.
/// Writes to a `std::FILE*` using `std::print()` (C++23).
template<Formattable T, Formattable... Rest>
void PrintFields(std::FILE* file, std::string_view sep,
                 const T& firstArg, const Rest&... rest)
{
  detail::print_fmt(file, firstArg);
  (detail::print_fmt(file, sep, rest), ...);
}

/// Print no fields separated by sep, no newline.
void PrintFields(std::FILE*, const std::string_view) { }

/// Print std::tuple separated by sep, no newline.
template<Formattable... Ts>
void PrintFields(std::FILE* file, std::string_view sep,
                 const std::tuple<Ts...>& t)
{
  std::apply([&](const Ts&... elems) {
    PrintFields(file, sep, elems...);
  }, t);
}

/// Print std::pair separated by sep, no newline.
template<Formattable T1, Formattable T2>
void PrintFields(std::FILE* file, std::string_view sep,
                 const std::pair<T1, T2>& p)
{ PrintFields(file, sep, p.first, p.second); }

/// Print variadic fields separated by sep, followed by newline.
template<typename... Args>
void PrintlnFields(FILE* file, const std::string_view sep, const Args&... args)
{
  PrintFields(file, sep, args...);
  std::println(file);
}

/// Print fields from a range, separated by sep, no newline.
template<std::ranges::range Range>
requires Streamable<std::ranges::range_value_t<Range>>
void PrintFields(std::ostream& os, const std::string_view sep,
                 const Range& fields)
{
  auto it = fields.begin();
  const auto end = fields.end();
  if (it == end)
    return;
  detail::print_fmt(os, *it++);
  while (it != end)
    detail::print_fmt(os, sep, *it++);
}

/// Print variadic fields separated by sep, no newline.
template<Streamable T, Streamable... Rest>
void PrintFields(std::ostream& os, const std::string_view sep,
                 const T& firstArg, const Rest&... rest)
{
  detail::print_fmt(os, firstArg);
  (detail::print_fmt(os, sep, rest), ...);
}

/// Print no fields separated by sep, no newline.
void PrintFields(std::ostream&, const std::string_view) { }

/// Print std::tuple separated by sep, no newline.
template<Streamable... Ts>
void PrintFields(std::ostream& os, std::string_view sep,
                 const std::tuple<Ts...>& t)
{
  std::apply([&](const Ts&... elems) {
    PrintFields(os, sep, elems...);
  }, t);
}

/// Print std::pair separated by sep, no newline.
template<Streamable T1, Streamable T2>
void PrintFields(std::ostream& os, std::string_view sep,
                 const std::pair<T1, T2>& p)
{ PrintFields(os, sep, p.first, p.second); }

/// Print variadic fields separated by sep, followed by newline.
template<typename... Args>
void PrintlnFields(std::ostream& os, const std::string_view sep,
                   const Args&... args)
{
  PrintFields(os, sep, args...);
  os << '\n';
}

/// Convenience overloads that write to either stdout or std::cout.

void PrintFields(std::string_view sep) { }

/// Print variadic fields separated by `sep`, no newline.
template<typename T, typename... Rest>
void PrintFields(std::string_view sep, const T& firstArg, const Rest&... rest) {
  detail::EnsureSync();
  detail::print_fmt(firstArg);
  (detail::print_fmt(sep, rest), ...);
}

template<std::ranges::range Range>
requires Formattable<std::ranges::range_value_t<Range>>
void PrintFields(const std::string_view sep, const Range& fields)
{ PrintFields(stdout, sep, fields); }

template<std::ranges::range Range>
requires JustStreamable<std::ranges::range_value_t<Range>>
void PrintFields(const std::string_view sep, const Range& fields) {
  detail::EnsureSync();
  PrintFields(std::cout, sep, fields);
}

template<typename... Ts>
void PrintFields(std::string_view sep, const std::tuple<Ts...>& t) {
  std::apply([&](const Ts&... elems) {
    PrintFields(sep, elems...);
  }, t);
}

template<typename T1, typename T2>
void PrintFields(std::string_view sep, const std::pair<T1, T2>& p)
{ PrintFields(sep, p.first, p.second); }

template<typename... Args>
void PrintlnFields(const std::string_view sep, const Args&... args) {
  PrintFields(sep, args...);
  std::println();
}

} // tjg
