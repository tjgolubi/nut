// Copyright 2023 Terry Golubiewski, all rights reserved.

#include <string>
#include <ranges>
#include <algorithm>
#include <optional>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <cctype>
#include <cstdlib>

struct Ingredient {
  std::string name;
  int g = 0;
  int ml = 0;
  int cal = 0;
  int prot = 0;
  int fat = 0;
  int carb = 0;
  int fiber = 0;
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
  output << setw(4) << ingred.g
	 << setw(4) << ingred.ml
	 << setw(4) << ingred.cal
	 << setw(4) << ingred.prot
	 << setw(4) << ingred.fat
	 << setw(4) << ingred.carb
	 << setw(4) << ingred.fiber
	 << ' ' << ingred.name;
  return output;
} // << Ingredient

std::vector<Ingredient> Ingredients;

auto Compare(const Ingredient& ingred, const std::string& name)
{ return (ingred.name <=> name); }

auto FindIngredient(const std::vector<Ingredient>& ingredients,
                    const std::string& name)
  -> std::optional<Ingredient>
{
  std::optional<Ingredient> rval;
  auto r = std::ranges::equal_range(ingredients, name,
                                    std::ranges::less {}, &Ingredient::name);
  if (!r.empty())
    rval = r.back();
  return rval;
}

auto ReadIngredients() {
  std::vector<Ingredient> rval;
  auto input = std::ifstream("ingred.txt");
  if (!input)
    return rval;
  Ingredient ingred;
  while (input >> std::ws) {
    {
      auto c = input.peek();
      if (c == '#') {
	input.ignore(1024, '\n');
	continue;
      }
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

const std::map<std::string, std::string> Abbrev = {
  { "c", "cup" },
  { "t", "tsp" },
  { "T", "tbsp" },
  { "each", "ea" },
  { "pint", "pt" },
  { "quart", "qt" },
  { "gallon", "gal" },
  { "gram" , "g" },
  { "liter", "l" }
}; // Abbrev

struct Line {
  double value = 0;
  std::string unit;
  std::string name;
}; // Line

auto Ratio(const Ingredient& ingred, const std::string& unit, double value)
  -> double
{
  if (unit == "ea")
    return value;
  if (ingred.ml != 0) {
    for (auto& it : Volumes) {
      if (unit == it.unit)
	return value * it.ml / ingred.ml;
    }
  }
  if (ingred.g != 0) {
    for (auto& it : Weights) {
      if (unit == it.unit)
	return value * it.g / ingred.g;
    }
  }
  return 0;
} // Ratio

auto ToLower(const std::string& str) {
  auto lwr = str;
  for (auto& c: lwr)
    c = tolower(c);
  return lwr;
}

int main() {
  const auto& ingredients = ReadIngredients();
  std::cout << "Read " << ingredients.size() << " ingredients." << std::endl;
  Line line;
  auto total = Ingredient("Total");
  while (std::cin) {
    std::cin >> line.value >> line.unit;
    std::getline(std::cin >> std::ws, line.name);
    if (!std::cin)
      break;
    auto ingred = FindIngredient(ingredients, ToLower(line.name))
      .value_or(Ingredient(line.name));
    auto iter = Abbrev.find(line.unit);
    auto unit = (iter != Abbrev.end()) ? iter->second : ToLower(line.unit);
    ingred.scale(Ratio(ingred, unit, line.value));
    using std::setw;
    std::cout
      << "g="    << setw(3) << ingred.g
      << " cal=" << setw(4) << ingred.cal
      << " p="   << setw(3) << ingred.prot
      << " f="   << setw(3) << ingred.fat
      << " c="   << setw(3) << ingred.carb
      << " fb="  << setw(3) << ingred.fiber
      << ' ' << setw(3) << line.value
      << ' ' << line.unit << ' ' << line.name << std::endl;
    total += ingred;
  }
  std::cout << "\nTotals:"
       << " g="   << total.g
       << " cal=" << total.cal
       << " p="   << total.prot
       << " f="   << total.fat
       << " c="   << total.carb
       << " fb="  << total.fiber
       << std::endl;
  return EXIT_SUCCESS;
} // main
