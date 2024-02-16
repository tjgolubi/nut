// Copyright 2024 Terry Golubiewski, all rights reserved.

#include <gsl/gsl>

#include <regex>
#include <ranges>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <cmath>
#include <cstring>
#include <cctype>
#include <cstdlib>

namespace rng = std::ranges;
using namespace std::string_literals;

class PrecSaver {
  std::ostream& os;
  std::streamsize prec;
  std::ios::fmtflags flags;
public:
  explicit PrecSaver(std::ostream& os_)
    : os(os_), prec{os_.precision()}, flags{os_.flags()} { }
  PrecSaver(std::ostream& os_, int p)
    : os(os_), prec{os_.precision(p)}, flags{os_.flags()} { }
  ~PrecSaver() {
    os.precision(prec);
    os.flags(flags);
  }
}; // PrecSaver

std::string ToLower(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  auto to_lower = [](unsigned char c) -> unsigned char
    { return std::tolower(c); };
  rng::transform(str, std::back_inserter(result), to_lower);
  return result;
}

void TrimLeadingWs(std::string& str)
{ str.erase(0, str.find_first_not_of(" \t\n\r\f\v")); }

void TrimTrailingWs(std::string& str) {
  auto i = str.find_last_not_of(" \t\n\r\f\v");
  if (i != std::string::npos && ++i < str.size())
    str.erase(i);
}

bool ContainsAny(const std::string& str1, const std::string& str2)
{ return (str1.find_first_of(str2) != std::string::npos); }

bool Contains(const std::string& str1, const std::string& str2)
{ return (str1.find(str2) != std::string::npos); }

bool Contains(const std::string& str1, gsl::czstring str2)
{ return (str1.find(str2) != std::string::npos); }

bool Contains(const std::string& str, char ch)
{ return (str.find(ch) != std::string::npos); }

bool Contains(gsl::czstring str, char ch)
{ return (std::strchr(str, ch) != nullptr); }

const std::map<std::string, std::string> FractionMap = {
  { "¼", "1/4" },
  { "½", "1/2" },
  { "¾", "3/4" },
  { "⅓", "1/3" },
  { "⅔", "2/3" },
  { "⅛", "1/8" },
  { "⅜", "3/8" },
  { "⅝", "5/8" },
  { "⅞", "7/8" }
}; // FractionMap

std::string SubstFraction(const std::string_view& str) {
  if (str.empty())
    return str;
  for (const auto& s: FractionMap) {
    auto i = str.find(s.first);
    if (i == std::string::npos)
      continue;
    std::string rval;
    if (i != 0) {
      rval = str.substr(0, i);
      if (std::isdigit(str[i-1]))
	rval += ' ';
    }
    rval += s.second;
    i += s.first.size();
    if (i < str.size() && std::isdigit(str[i]))
      rval += ' ';
    return rval += str.substr(i);
  }
  return str;
} // SubstFraction

float Value(const std::string_view& arg) {
  if (arg.empty())
    return 0;
  auto str = SubstFraction(arg);
  if (Contains(str, '.') || !ContainsAny(str, "-/ ")) {
    std::size_t pos = 0;
    auto rval = 0.0f;
    try { rval = std::stof(str, &pos); }
    catch (...) { return 0.0f; }
    if (pos != str.size())
      return 0.0f;
    return rval;
  }
  std::istringstream iss(str);
  int base = 0;
  iss >> base;
  if (!iss || base < 0)
    return 0;
  switch (iss.peek()) {
    case '/': {
      iss.ignore();
      int den = 0;
      iss >> den;
      if (!iss || !iss.eof() || den <= base)
	return 0;
      return float(base) / den;
    }
    case ' ':
    case '-':
      iss.ignore();
      break;
    default:
      return 0;
  }
  int num = 0;
  iss >> num;
  if (!iss || num <= 0 || iss.peek() != '/')
    return 0;
  iss.ignore();
  int den = 0;
  iss >> den;
  if (!iss || den <= num || !iss.eof())
    return 0;
  return base + float(num) / den;
} // Value

constexpr float CUP = 236.59f;

const std::map<std::regex, float> Volumes = {
  { "^cc\\b",                    1  },
  { "^cubic[ -]centimeters?\\b", 1  },
  { "^cubic[ -]cms?\\b",         1  },
  { "^cubic[ -]inchs?\\b",   16.39  },
  { "^cups?\\b",             CUP    },
  { "^fl[ -]?oz\\b",         CUP/8  },
  { "^gallons?\\b",          CUP*16 },
  { "^gals?\\b"              CUP*16 },
  { "^liters?\\b",           1000   },
  { "^milliliters?\\b",      1      },
  { "^pints?\\b",            CUP*2  },
  { "^pt\\b",                CUP*2  },
  { "^qt\\b",                CUP*4  },
  { "^quarts?\\b",           CUP*4  },
  { "^shots?\\b",            44     },
  { "^tablespoons?\\b",      CUP/16 },
  { "^tbsp\\b",              CUP/16 },
  { "^teaspoons?\\b",        CUP/48 },
  { "^tsp\\b",               CUP/48 }
}; // Volumes

auto FindVolume(const std::string_view& desc) {
  for (const auto& [re, ml] : Volumes) {
    if (std::regex_match(re, desc)
      return ml;
  }
  return 0.0;
} // FindVolume

Line Parse(const std::string& str) {
  Line line;
  std::istringstream input(str);
  input >> line.value;
  if (!input)
    return line;
  input >> std::ws;
  if (input.peek() != '(') {
    input >> line.unit;
    if (!input)
      return line;
    if (!ContainsAny(line.value, ".-/")
	&& !FractionMap.contains(line.value)
	&& !line.unit.empty())
    {
      if (std::isdigit(line.unit[0]) && Contains(line.unit, '/')
	|| FractionMap.contains(line.unit))
      {
	line.value += ' ';
	line.value += line.unit;
	input >> line.unit;
	if (!input)
	  return line;
      }
    }
    input >> std::ws;
  }
  if (input.peek() == '(') {
    input.ignore();
    std::getline(input >> std::ws, line.weight, ')');
    TrimTrailingWs(line.weight);
    input >> std::ws;
  }
  std::getline(input, line.name);
  return line;
} // Parse

int main() {
  try {
  }
  catch (const std::exception& x) {
    std::cout << "standard exception: " << x.what() << std::endl;
  }
  return EXIT_FAILURE;
} // main
