// Copyright 2023-2024 Terry Golubiewski, all rights reserved.
#include "Atwater.h"

#include "To.h"

#include <ranges>
#include <array>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace rng = std::ranges;

#if defined(_LIBCPP_CLANG_VER) && (_LIBCPP_CLANG_VER <= 1700)
namespace std::ranges {

template<typename R>
constexpr bool contains(const R& r, const auto& x)
{ return (find(r, x) != end(r)); }

} // std::ranges
#endif

float Atwater::kcal(const Nutrition& nutr) const {
  float rval = prot * nutr.prot + fat * nutr.fat + alcohol * nutr.alcohol;;
  if (fiber == 0.0f)
    return rval + carb * nutr.carb;
  return rval + carb * (nutr.carb - nutr.fiber) + fiber * nutr.fiber;
}  // kcal

std::istream& SetFail(std::istream& is) {
  is.setstate(std::ios_base::failbit);
  return is;
} // SetFail

const std::map<std::string_view, Atwater> Atwater::Names = {
  { "egg",         { 4.36, 9.02, 3.68 } },
  { "gelatin",     { 3.90, 9.02, 3.87 } },
  { "glycogen",    { 4.27, 9.02, 4.11 } },
  { "meat",        { 4.27, 9.02, 3.82 } },
  { "fish",        { 4.27, 9.02, 3.82 } },
  { "gland",       { 4.27, 9.02, 3.87 } },
  { "tongue",      { 4.27, 9.02, 4.11 } },
  { "shellfish",   { 4.27, 9.02, 4.11 } },
  { "milk",        { 4.27, 8.97, 3.87 } },
  { "oil",         { 4.27, 8.84, 3.87 } },
  { "fruit",       { 3.36, 8.37, 3.60 } },
  { "juice",       { 3.36, 8.37, 3.92 } },
  { "lemon",       { 3.36, 8.37, 2.48 } },
  { "lemon juice", { 3.36, 8.37, 2.70 } },
  { "barley",      { 3.55, 8.37, 3.95 } },
  { "dark buckwheat flour",  { 3.37, 8.37, 3.78 } },
  { "light buckwheat flour", { 3.55, 8.37, 3.95 } },
  { "whole cornmeal",    { 2.73, 8.37, 4.03 } },
  { "degermed cornmeal", { 3.46, 8.37, 4.16 } },
  { "dextrin",     { 3.00, 8.37, 4.03 } },
  { "pasta",       { 3.91, 8.37, 4.12 } },
  { "oat",         { 3.46, 8.37, 4.12 } },
  { "brown rice",  { 3.41, 8.37, 4.12 } },
  { "white rice",  { 3.82, 8.37, 4.16 } },
  { "dark rye flour",   { 2.96, 8.37, 3.78 } },
  { "whole rye flour",  { 3.05, 8.37, 3.86 } },
  { "medium rye flour", { 3.23, 8.37, 3.99 } },
  { "light rye flour",  { 3.41, 8.37, 4.07 } },
  { "whole sorghum",    { 0.91, 8.37, 4.03 } },
  { "light sorghum",    { 2.28, 8.37, 4.07 } },
  { "whole wheat flour",  { 3.59, 8.37, 3.78 } },
  { "wheat flour",        { 3.78, 8.37, 3.95 } },
  { "patent wheat flour", { 4.05, 8.37, 4.12 } },
  { "wheat",       { 3.59, 8.37, 3.78 } },
  { "wheat bran",  { 1.82, 8.37, 2.35 } },
  { "cereal",      { 3.87, 8.37, 4.12 } },
  { "bread",       { 3.9,  8.7,  4.1  } },
  { "wild rice",   { 3.55, 8.37, 3.95 } },
  { "legume",      { 3.47, 8.37, 4.07 } },
  { "sucrose",     { 3.95, 8.37, 3.87 } },
  { "glucose",     { 3.95, 8.37, 3.68 } },
  { "mushroom",    { 2.62, 8.37, 3.48 } },
  { "potato",      { 2.78, 8.37, 4.03 } },
  { "root",        { 2.78, 8.37, 3.84 } },
  { "mustard",     { 3.47, 8.37, 3.34 } },
  { "vegetable",   { 2.44, 8.37, 3.57 } },
  { "cocoa",       { 1.83, 8.37, 1.33 } },
  { "vinegar",     { 3.95, 8.37, 2.40 } },
  { "yeast",       { 3.00, 8.37, 3.35 } },
  { "general",     { 4.00, 9.00, 4.00 } }
}; // Atwater::Names

auto GenerateReverseMap() {
  std::map<Atwater, std::string> rval;
  for (const auto& [name, atwater]: Atwater::Names)
    rval[atwater] = name;
  return rval;
} // GenerateReverseMap

const std::map<Atwater, std::string> ReverseMap = GenerateReverseMap();

using SynMap = std::map<std::string_view, std::string_view>;
static const SynMap Synonyms = {
  { "fish",    "meat" },
  { "poultry", "meat" },
  { "light wheat flour",  "patent wheat flour"   },
  { "medium wheat flour", "wheat flour"          },
  { "dark wheat flour",   "whole wheat flour"    },
  { "honey",  "glucose" },
  { "sugar",  "sucrose" },
  { "bean",   "legume"  },
  { "nut",    "legume"  },
  { "seed",   "legume"  },
  { "brain",  "gland"   },
  { "heart",  "gland"   },
  { "liver",  "gland"   },
  { "kidney", "gland"  },
  { "lime",   "lemon"   },
  { "lime juice", "lemon juice" },
  { "cornmeal", "whole cornmeal" },
  { "sorghum",  "whole sorghum" },
  { "millet",   "whole sorghum" },
  { "whole wheat pasta", "whole wheat flour" },
  { "grain",    "cereal"        },
  { "beer",     "cereal" },
  { "wine",     "juice"  }
}; // Synonyms

std::string Atwater::values_str(std::string_view delim) const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  oss << prot << delim << fat << delim << carb;
  if (fiber != 0.0f)
    oss << delim << fiber;
  return oss.str();
} // values_str

std::string Atwater::str(std::string_view delim) const {
  auto iter = ReverseMap.find(*this);
  if (iter != ReverseMap.end())
    return iter->second;
  return values_str(delim);
} // str

Atwater::Atwater(const std::string_view& str) {
  static const std::range_error error{"Invalid Atwater initialization string"};
  if (str.empty()) {
    *this = Atwater{};
    return;
  }
  if (auto iter = Names.find(str); iter != Names.end()) {
    *this = iter->second;
    return;
  }
  if (auto iter = Synonyms.find(str); iter != Synonyms.end()) {
    *this = Atwater(iter->second);
    return;
  }
  auto result = rng::views::split(str, ' ');
  if (!result || rng::contains(result.front(), ','))
    result = rng::views::split(str, ',');
  if (!result)
    throw error;
  *this = Atwater{};
  std::array<float*, 4> m {
    &prot, &fat, &carb, &fiber
  };
  auto iter = std::begin(m);
  for (auto v: result) {
    auto x = To<float>(v);
    if (iter == std::end(m))
      throw error;
    **iter = x;
    ++iter;
  }
} // ctor(string_view)

std::istream& operator>>(std::istream& is, Atwater& atwater) {
  is >> std::ws;
  if (is.peek() != '[')
    return SetFail(is);
  is.ignore();
  is >> std::ws;
  atwater = Atwater();
  std::string name;
  if (!std::getline(is, name, ']'))
    return is;
  try { atwater = Atwater(name); }
  catch (...) { SetFail(is); }
  return is;
} // operator >> Atwater

std::ostream& operator<<(std::ostream& os, const Atwater& atwater)
{ return os << '[' << atwater.str(" ") << ']'; }

