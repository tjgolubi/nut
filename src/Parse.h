/// @file Parse.h
/// @brief Utilities for parsing delimited text into structured fields.
/// @copyright Copyright 2023-2025 Terry Golubiewski, all rights reserved.

#ifndef PARSE_H
#define PARSE_H
#pragma once

#include <gsl/gsl>

#include <string>
#include <vector>
#include <array>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>

/// @brief Parses a delimited string into a vector of fields.
///
/// Splits the input line based on the provided delimiter,
/// respecting quoted substrings.
/// - Doubled quote characters are interpreted as escaped quotes
///   (e.g., "" → ").
/// - Leading/trailing whitespace and outer quotes are removed
///   from each field.
/// - Tabs inside fields are converted to spaces.
///
/// The vector `row` is resized as needed. Excess trailing fields
/// are discarded. Throws std::runtime_error if a field separator
/// is missing where expected.
///
/// @param line The input line to parse.
/// @param row Output vector to receive parsed fields.
/// @param sep Field delimiter (default: tab).
/// @param quote Quote character (default: none).
/// @param escape Escape character for quoted strings
///               (default: none).
/// @throw std::runtime_error if a separator is missing between
///        fields.
void Parse(const std::string& line, std::vector<std::string>& row,
           const char sep='\t', const char quote='\0', const char escape='\0');

/// @brief Parses a tab-separated line.
inline
void ParseTsv(const std::string& line, std::vector<std::string>& row)
  { Parse(line, row, '\t'); }

/// @brief Parses a comma-separated line, recognizing quoted fields.
inline
void ParseCsv(const std::string& line, std::vector<std::string>& row)
  { Parse(line, row, ',', '"', '\\'); }

/// @brief Parses a caret-delimited line with tildes as quote chars.
inline
void ParseTxt(const std::string& line, std::vector<std::string>& row)
  { Parse(line, row, '^', '~', '\\'); }

/// @brief Vector of strings indexable by enum.
///
/// Assumes the enum `E` defines a final enumerator named `end`
/// representing its size. Provides bounds-checked and operator[]
/// access using enum values.
///
/// @tparam E Enum type with a defined `end` sentinel.
template<class E>
struct ParseVec: public std::vector<std::string> {
  using base_type = std::vector<std::string>;
  const std::string& at(E e) const { return base_type::at(gsl::index(e)); }
  const std::string& operator[](E e) const { return this->at(e); }
}; // ParseVec

/// @brief Stream insertion for ParseVec.
///
/// Outputs each field enclosed in quotes, delimited by spaces,
/// within angle brackets. Example: < "field1" "field2" >
///
/// @tparam E Enum used to define field count and access.
template <class E>
std::ostream& operator<<(std::ostream& os, const ParseVec<E>& v) {
  os << '<';
  for (const auto& s: v)
    os << ' ' << std::quoted(s);
  return os << " >";
} // << ParseVec

/// @brief Parses a delimited string into a ParseVec.
///
/// Tokenizes the input using the given separator, quote, and
/// escape characters. Throws if the number of tokens does not
/// match the enum-defined size, an enumerator named 'end'..
///
/// @tparam E Enum used for indexing.
/// @param v Output ParseVec to populate.
/// @param str Input string to parse.
/// @param sep Delimiter character.
/// @param quote Quote character.
/// @param escape Escape character.
/// @return Reference to populated ParseVec.
/// @throw std::runtime_error if field count mismatches.
template<class E>
auto Parse(ParseVec<E>& v, const std::string& str,
           const char sep, const char quote='\0', const char escape='\0')
  -> ParseVec<E>&
{
  std::istringstream iss(str);
  std::string s;
  v.clear();
  if (iss.peek() == quote) {
    iss >> std::quoted(s, quote, escape);
  }
  else {
    std::getline(iss, s, sep);
    if (!iss.eof())
      iss.unget();
  }
  if (iss)
    v.push_back(s);
  char c = '\0';
  while (iss.get(c) && c == sep) {
    if (iss.peek() == quote) {
      if (iss >> std::quoted(s, quote, escape))
        v.push_back(s);
    }
    else {
      std::getline(iss, s, sep);
      v.push_back(s);
      if (!iss.eof())
        iss.unget();
    }
  }
  if (v.size() != static_cast<ParseVec<E>::size_type>(E::end))
    throw std::runtime_error{"Parse: invalid number of columns"};
  return v;
} // Parse

/// @brief Parses a tab-separated line into a ParseVec.
///
/// Convenience wrapper using tab as the field separator.
///
/// @tparam E Enum for indexing.
/// @param v ParseVec to populate.
/// @param str Input line.
/// @return Reference to populated ParseVec.
template<class E>
auto ParseTsv(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, '\t'); }

/// @brief Parses a CSV-formatted line into a ParseVec.
///
/// Uses ',' as the separator and '"' for quoted fields.
///
/// @tparam E Enum for indexing.
/// @param v ParseVec to populate.
/// @param str Input line.
/// @return Reference to populated ParseVec.
template<class E>
auto ParseCsv(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, ',', '"', '\\'); }

/// @brief Parses a caret-delimited, tilde-quoted line into a ParseVec.
///
/// @tparam E Enum for indexing.
/// @param v ParseVec to populate.
/// @param str Input line.
/// @return Reference to populated ParseVec.
template<class E>
auto ParseTxt(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, '^', '~'); }

/// @brief Validates that parsed headings match expected strings.
///
/// Compares each value in the ParseVec to a corresponding string_view.
/// Throws std::runtime_error on the first mismatch.
///
/// @tparam Idx Enum used for indexing.
/// @tparam N Size of expected headings array.
/// @param v Parsed headings.
/// @param headings Expected string_view labels.
/// @throw std::runtime_error on mismatch.
template<class Idx, std::size_t N>
void CheckHeadings(const ParseVec<Idx>& v,
                   const std::array<std::string_view, N>& headings)
{
  const auto size = gsl::index{headings.size()};
  for (gsl::index i = 0; i != size; ++i) {
    if (v.at(gsl::narrow_cast<Idx>(i)) != headings[i])
      throw std::runtime_error{"Invalid column headings"};
  }
} // CheckHeadings

#endif
