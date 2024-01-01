// Copyright 2023 Terry Golubiewski, all rights reserved.
#include "Atwater.h"

#include <map>
#include <iomanip>

double Atwater::kcal(const Nutrition& nutr) const {
  double rval = prot * nutr.prot + fat * nutr.fat;
  if (nutr.fiber < 0.0)
    return rval + carb * nutr.carb + alcohol * -nutr.fiber;
  if (fiber == 0.0)
    return rval + carb * nutr.carb;
  return rval + carb * (nutr.carb - nutr.fiber) + fiber * nutr.fiber;
}  // kcal

std::ostream& operator<<(std::ostream& os, const Atwater& atwater) {
  os << '[' << atwater.prot << ' ' << atwater.fat << ' ' << atwater.carb;
  if (atwater.fiber != 0.0)
    os << ' ' << atwater.fiber;
  return os << ']';
} // operator << Atwater

std::istream& SetFail(std::istream& is) {
  is.setstate(std::ios_base::failbit);
  return is;
} // SetFail

using Map = std::map<std::string, Atwater>;

static const Map Names = {
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
}; // Names

using SynMap = std::map<std::string, std::string>;
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

std::istream& operator>>(std::istream& is, Atwater& atwater) {
  is >> std::ws;
  if (is.peek() != '[')
    return SetFail(is);
  is.ignore();
  is >> std::ws;
  atwater = Atwater();
  if (!std::isdigit(is.peek())) {
    std::string name;
    std::getline(is, name, ']');
    if (!is)
      return is;
    if (auto iter = Synonyms.find(name); iter != Synonyms.end())
      name = iter->second;
    if (auto iter = Names.find(name); iter != Names.end()) {
      atwater = iter->second;
      return is;
    }
    return SetFail(is);
  }
  is >> atwater.prot >> atwater.fat >> atwater.carb >> std::ws;
  if (!is)
    return is;
  if (is.peek() != ']')
    is >> atwater.fiber >> std::ws;
  else
    atwater.fiber = 0.0;
  if (is.peek() != ']')
    return SetFail(is);
  return is.ignore();
} // operator >> Atwater
