// Copyright 2023 Terry Golubiewski, all rights reserved.

#include <ranges>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <cmath>
#include <cctype>
#include <cstdlib>

class PrecSaver {
private:
  std::ostream& os;
  int prec;
public:
  explicit PrecSaver(std::ostream& output)
    : os(output), prec(output.precision()) { }
  PrecSaver(std::ostream& output, int p)
    : os(output), prec(output.precision(p)) { }
  ~PrecSaver() { os.precision(prec); }
}; // PrecSaver

struct Ingredient {
  std::string name;
  double g     = 0.0;
  double ml    = 0.0;
  double cal   = 0.0;
  double prot  = 0.0;
  double fat   = 0.0;
  double carb  = 0.0;
  double fiber = 0.0;
  Ingredient() : name() { }
  explicit Ingredient(const std::string& n) : name(n) { }
  void scale(double ratio) {
    g     *= ratio;
    ml    *= ratio;
    cal   *= ratio;
    prot  *= ratio;
    fat   *= ratio;
    carb  *= ratio;
    fiber *= ratio;
  }
  Ingredient& operator+=(const Ingredient& rhs) {
    g     += rhs.g;
    ml    += rhs.ml;
    cal   += rhs.cal;
    prot  += rhs.prot;
    fat   += rhs.fat;
    carb  += rhs.carb;
    fiber += rhs.fiber;
    return *this;
  }
  bool operator==(const Ingredient& rhs) const = default;
  auto operator<=>(const Ingredient& lhs) const = default;
}; // Ingredient

std::ostream& operator<<(std::ostream& output, const Ingredient& ingred) {
  using std::setw;
  using std::round;
  PrecSaver precSaver(output, 5);
  output << setw(4) << round(ingred.g)
	 << setw(4) << round(ingred.ml)
	 << setw(4) << round(ingred.cal)
	 << setw(6) << ingred.prot
	 << setw(6) << ingred.fat
	 << setw(6) << ingred.carb
	 << setw(6) << ingred.fiber
	 << ' ' << ingred.name;
  return output;
} // << Ingredient

std::vector<Ingredient> Ingredients;

#if 0
auto Compare(const Ingredient& ingred, const std::string& name)
{ return (ingred.name <=> name); }
#endif

auto FindIngredient(const std::vector<Ingredient>& ingredients,
                    const std::string& name)
  -> std::optional<Ingredient>
{
  auto r = std::ranges::lower_bound(ingredients, name,
                                    std::ranges::less {}, &Ingredient::name);
  if (r == ingredients.end() || r->name != name)
    return std::nullopt;

  return *r;
} // FindIngredient

auto ReadIngredients() {
  std::string str;
  std::vector<Ingredient> rval;
  auto input = std::ifstream("ingred.txt");
  if (!input)
    return rval;
  Ingredient ingred;
  while (input >> std::ws) {
    if (input.peek() == '#') {
      input.ignore(1024, '\n');
      continue;
    }
    ingred = Ingredient();
    input >> ingred.g >> ingred.ml >> ingred.cal
	  >> ingred.prot >> ingred.fat >> ingred.carb >> ingred.fiber;
    if (input && std::getline(input >> std::ws, ingred.name))
      rval.push_back(ingred);
  }
  std::sort(rval.begin(), rval.end()); // todo: use std::ranges::sort
  return rval;
} // ReadIngredients

auto ToLower(const std::string& str) {
  auto lwr = str;
  for (auto& c: lwr)
    c = tolower(c);
  return lwr;
}

const std::map<std::string, std::string> ValSyn = {
  { "1/8", "0.125" },
  { "1/4", "0.25" },
  { "1/3", "0.333" },
  { "1/2", "0.5" },
  { "2/3", "0.667" },
  { "3/4", "0.75" }
}; // ValSyn

const std::map<std::string, std::string> UnitSyn = {
  { "each",    "ea"   },
  { "piece",   "ea"   },
  { "pieces",  "ea"   },
  { "#",       "lb"   },
  { "T",       "tbsp" },
  { "c",       "cup"  },
  { "cups",    "cup"  },
  { "gallon",  "gal"  },
  { "gallons", "gal"  },
  { "gram" ,   "g"    },
  { "grams",   "g"    },
  { "liter",   "l"    },
  { "liters",  "l"    },
  { "ounce",   "oz"   },
  { "ounces",  "oz"   },
  { "pint",    "pt"   },
  { "pints",   "pt"   },
  { "pound",   "lb"   },
  { "pounds",  "lb"   },
  { "quart",   "qt"   },
  { "quarts",  "qt"   },
  { "shots",   "shot" },
  { "t",       "tsp"  },
  { "tbsps",   "tbsp" },
  { "tsps",    "tsp"  }
}; // UnitSyn

auto FindValue(const std::string& value) {
  auto it = ValSyn.find(value);
  return std::stod((it != ValSyn.end()) ? it->second : value);
}

auto FindUnit(const std::string& unit) {
  auto it = UnitSyn.find(unit);
  return (it != UnitSyn.end()) ? it->second : ToLower(unit);
}

struct Volume {
  std::string unit;
  double ml = 0;
};

const std::vector<Volume> Volumes = {
  { "ml",     1 },
  { "l",   1000 },
  { "tsp",    4.9289 },
  { "tbsp",  14.7868 },
  { "floz",  29.5735 },
  { "shot",  44.3603 },
  { "cup",  236.5882 },
  { "pt",   473.1765 },
  { "qt",   946.3529 },
  { "gal", 3785.4118 }
}; // Volumes

auto FindVolume(const std::string& unit) {
  for (auto& it : Volumes) {
    if (it.unit == unit)
      return it.ml;
  }
  return 0.0;
} // FindVolume

struct Weight {
  std::string unit;
  double g = 0;
};

const std::vector<Weight> Weights = {
  { "g",     1 },
  { "kg", 1000 },
  { "oz",  28.3495 },
  { "lb", 453.5924 }
}; // Weights

auto FindWeight(const std::string& unit) {
  for (auto& it : Weights) {
    if (it.unit == unit)
      return it.g;
  }
  return 0.0;
} // FindWeight

auto Ratio(const Ingredient& ingred, const std::string& unit, double value) {
  if (unit == "ea" && ingred.g < 0.0)
    return value;
  if (ingred.ml != 0) {
    auto v = FindVolume(unit);
    if (v != 0.0)
      return value * v / ingred.ml;
  }
  if (ingred.g != 0) {
    auto v = FindWeight(unit);
    if (v != 0.0)
      return value * v / std::abs(ingred.g);
  }
  return 0.0;
} // Ratio

void Subst(std::string& str1, const std::string& str2, const std::string& str3)
{
  auto idx = str1.find(str2);
  if (idx != std::string::npos)
    str1.replace(idx, str2.length(), str3);
} // Subst

int main() {
  using std::cout;
  const auto& ingredients = ReadIngredients();
  cout << "Read " << ingredients.size() << " ingredients." << std::endl;

  struct Line {
    std::string value;
    std::string unit;
    std::string name;
  }; // Line

  Line line;
  auto total = Ingredient("Total");
  while (std::cin) {
    std::cin >> std::ws;
    if (std::cin.peek() == '#') {
      std::cin.ignore(1024, '\n');
      continue;
    }
    std::cin >> line.value >> line.unit;
    std::getline(std::cin >> std::ws, line.name);
    if (!std::cin)
      break;
    auto name = ToLower(line.name);
    { // trim punctuation
      const auto punct = std::string("!#$()*+,./:;<=>?@[]^{|}~");
      auto i = name.find_first_of(punct);
      if (i != std::string::npos)
	name.erase(i);
    }
    { // trim whitespace
      const auto ws = std::string(" \t\n\r\f\v");
      auto i = name.find_last_not_of(ws);
      if (i != std::string::npos && ++i < name.size())
	name.erase(i);
    }
    { // substitute common synonyms
      Subst(name, "diced", "chopped");
      Subst(name, "dry",   "dried");
    }
    auto ingred = FindIngredient(ingredients, name);
    if (!ingred && !name.empty() && name.back() == 's') {
      name.pop_back();
      ingred = FindIngredient(ingredients, name);
      if (!ingred && !name.empty() && name.back() == 'e') {
	name.pop_back();
	ingred = FindIngredient(ingredients, name);
	if (!ingred && !name.empty() && name.back() == 'i') {
	  name.back() = 'y';
	  ingred = FindIngredient(ingredients, name);
	}
      }
    }
    if (!ingred)
      ingred = Ingredient(line.name);
    auto& ing = *ingred;
    auto value = FindValue(line.value);
    auto unit  = FindUnit(line.unit);
    ing.scale(Ratio(ing, unit, value));
    ing.g = std::abs(ing.g);
    using std::setw;
    using std::round;
    cout
      << "g="    << setw(3) << round(ing.g)
      << " cal=" << setw(4) << round(ing.cal)
      << " p="   << setw(3) << round(ing.prot)
      << " f="   << setw(3) << round(ing.fat)
      << " c="   << setw(3) << round(ing.carb)
      << " fb="  << setw(3) << round(ing.fiber)
      << " : " << line.value << ' ' << line.unit << ' ' << line.name
      << std::endl;
    total += ing;
  }
  cout << "\nTotals:"
       << " g="   << round(total.g)
       << " cal=" << round(total.cal)
       << " p="   << round(total.prot)
       << " f="   << round(total.fat)
       << " c="   << round(total.carb)
       << " fb="  << round(total.fiber)
       << std::endl;
  return EXIT_SUCCESS;
} // main
