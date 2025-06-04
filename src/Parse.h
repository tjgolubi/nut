// Copyright 2023-2024 Terry Golubiewski, all rights reserved.
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

void Parse(const std::string& line, std::vector<std::string>& row,
	   const char sep='\t', const char quote='\0', const char escape='\0');

inline
void ParseTsv(const std::string& line, std::vector<std::string>& row)
  { Parse(line, row, '\t'); }

inline
void ParseCsv(const std::string& line, std::vector<std::string>& row)
  { Parse(line, row, ',', '"', '\\'); }

inline
void ParseTxt(const std::string& line, std::vector<std::string>& row)
  { Parse(line, row, '^', '~', '\\'); }

template<class E>
struct ParseVec: public std::vector<std::string> {
  using base_type = std::vector<std::string>;
  const std::string& at(E e) const { return base_type::at(gsl::index(e)); }
  const std::string& operator[](E e) const { return this->at(e); }
}; // ParseVec

template <class E>
std::ostream& operator<<(std::ostream& os, const ParseVec<E>& v) {
  os << '<';
  for (const auto& s: v)
    os << ' ' << std::quoted(s);
  return os << " >";
} // << ParseVec

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

template<class E>
auto ParseTsv(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, '\t'); }

template<class E>
auto ParseCsv(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, ',', '"', '\\'); }

template<class E>
auto ParseTxt(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, '^', '~'); }

template<class Idx, std::size_t N>
void CheckHeadings(const ParseVec<Idx>& v,
                   const std::array<std::string_view, N>& headings)
{
  for (gsl::index i = 0; i != headings.size(); ++i) {
    if (v.at(gsl::narrow_cast<Idx>(i)) != headings[i])
      throw std::runtime_error{"Invalid column headings"};
  }
} // CheckHeadings

#endif
