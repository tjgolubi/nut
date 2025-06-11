// Copyright 2023-2025 Terry Golubiewski, all rights reserved.

#include "Nutrition.h"

#include <mp-units/systems/usc.h>
#include <mp-units/systems/usc.h>

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

struct Ingredient {
  std::string name;
  Nutrition nutr;
  auto operator<=>(const Ingredient&) const = default;
}; // Ingredient

using NutrVec = std::vector<Ingredient>;

void ReadIngredients(NutrVec& ingredients) {
  gsl::czstring dir = std::getenv("INGRED_PATH");
  if (!dir)
    throw std::runtime_error("INGRED_PATH not set");
  const auto fname = dir + "/ingred.dat"s;
  std::ifstream input{fname, std::ios::binary};
  if (!input || !input.is_open())
    throw std::runtime_error(fname + ": cannot read"s);
  Ingredient ingr;
  while (std::getline(input, ingr.name, '\0')) {
    input.read(reinterpret_cast<char*>(&ingr.nutr), sizeof(ingr.nutr));
    ingr.nutr.fiber = std::max(Nutrition::Gram{}, ingr.nutr.fiber); // remove alcohol
    ingredients.push_back(ingr);
  }
  if (!rng::is_sorted(ingredients))
    throw std::runtime_error(fname + " is not sorted"s);
} // ReadIngredients

[[nodiscard]]
auto FindIngredient(const NutrVec& ingredients,
                    const std::string& name)
  -> std::optional<Nutrition>
{
  if (name.empty())
    return std::nullopt;

  auto i = rng::lower_bound(ingredients, name, {}, &Ingredient::name);
  if (i == ingredients.end() || i->name != name)
    return std::nullopt;

  return i->nutr;
} // FindIngredient

[[nodiscard]]
auto FindIngredientWithPlurals(const NutrVec& ingredients,
			       std::string name)
  -> std::optional<Nutrition>
{
  auto nutr = FindIngredient(ingredients, name);
  if (nutr || name.size() <= 1 || name.back() != 's')
    return nutr;
  name.pop_back();
  nutr = FindIngredient(ingredients, name);
  if (nutr || name.size() <= 1 || name.back() != 'e')
    return nutr;
  name.pop_back();
  nutr = FindIngredient(ingredients, name);
  if (nutr || name.size() <= 1 || name.back() != 'i')
    return nutr;
  name.back() = 'y';
  return FindIngredient(ingredients, name);
} // FindIngredientWithPlurals

[[nodiscard]]
unsigned char ToLower(unsigned char c) { return std::tolower(c); }

[[nodiscard]]
auto ToLower(const std::string& str) -> std::string {
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

[[nodiscard]]
bool ContainsAny(const std::string& str1, const std::string& str2)
{ return (str1.find_first_of(str2) != std::string::npos); }

[[nodiscard]]
bool Contains(const std::string& str1, const std::string& str2)
{ return (str1.find(str2) != std::string::npos); }

[[nodiscard]]
bool Contains(const std::string& str1, gsl::czstring str2)
{ return (str1.find(str2) != std::string::npos); }

[[nodiscard]]
bool Contains(const std::string& str, char ch)
{ return (str.find(ch) != std::string::npos); }

[[nodiscard]]
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

[[nodiscard]]
auto SubstFraction(const std::string& str) -> std::string {
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

[[nodiscard]]
double Value(const std::string& arg) {
  if (arg.empty())
    return 0;
  auto str = SubstFraction(arg);
  if (Contains(str, '.') || !ContainsAny(str, "-/ ")) {
    auto pos = std::size_t{0};
    auto rval = 0.0;
    try { rval = std::stod(str, &pos); }
    catch (...) { return 0; }
    if (pos != str.size())
      return 0;
    return rval;
  }
  auto iss = std::istringstream{str};
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
      return double(base) / den;
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
  return base + double(num) / den;
} // Value

const std::map<std::string, std::string> UnitSyn = {
  { "#",       "lb"   },
  { "T",       "tbsp" },
  { "c",       "cup"  },
  { "cups",    "cup"  },
  { "each",    "ea"   },
  { "gallon",  "gal"  },
  { "gallons", "gal"  },
  { "gram" ,   "g"    },
  { "grams",   "g"    },
  { "liter",   "l"    },
  { "liters",  "l"    },
  { "ounce",   "oz"   },
  { "ounces",  "oz"   },
  { "piece",   "ea"   },
  { "pieces",  "ea"   },
  { "pint",    "pt"   },
  { "pints",   "pt"   },
  { "pound",   "lb"   },
  { "pounds",  "lb"   },
  { "quart",   "qt"   },
  { "quarts",  "qt"   },
  { "shots",   "shot" },
  { "t",       "tsp"  },
  { "tablespoon",  "tbsp" },
  { "tablespoons", "tbsp" },
  { "tbsps",       "tbsp" },
  { "teaspoon",    "tsp"  },
  { "teaspoons",   "tsp"  },
  { "tsps",        "tsp"  }
}; // UnitSyn

namespace mp_units::si {

inline constexpr auto millilitre = milli<litre>;

} // mp_units::si

using namespace mp_units;

using Gram  = quantity<si::gram>;
using Litre = quantity<si::litre>;

[[nodiscard]]
auto FindUnit(const std::string& unit) {
  if (unit.empty())
    return std::string("ea");
  auto u = ToLower(unit);
  auto it = UnitSyn.find(u);
  return (it != UnitSyn.end()) ? it->second : u;
}

struct Volume {
  std::string unit;
  Litre vol;
};

const std::vector<Volume> Volumes = {
  { "ml",   1.0 * si::millilitre   },
  { "l",    1.0 * si::litre        },
  { "tsp",  1.0 * usc::teaspoon    },
  { "tbsp", 1.0 * usc::tablespoon  },
  { "floz", 1.0 * usc::fluid_ounce },
  { "shot", 1.0 * usc::shot        },
  { "cup",  1.0 * usc::cup         },
  { "pt",   1.0 * usc::pint        },
  { "qt",   1.0 * usc::quart       },
  { "gal",  1.0 * usc::gallon      }
}; // Volumes

[[nodiscard]]
auto FindVolume(const std::string& unit) {
  for (auto& it : Volumes) {
    if (it.unit == unit)
      return it.vol;
  }
  return Litre::zero();
} // FindVolume

struct Weight {
  std::string unit;
  Gram wt;
};

const std::vector<Weight> Weights = {
  { "g",  1.0 * si::gram     },
  { "kg", 1.0 * si::kilogram },
  { "oz", 1.0 * usc::ounce   },
  { "lb", 1.0 * usc::pound   }
}; // Weights

[[nodiscard]]
auto FindWeight(const std::string& unit) {
  for (auto& it : Weights) {
    if (it.unit == unit)
      return it.wt;
  }
  return Gram::zero();
} // FindWeight

namespace std {

template<auto T, typename Rep>
inline constexpr auto abs(quantity<T, Rep> x) {
  return (x >= quantity<T, Rep>::zero())
    ? x : -x;
}

} // std

[[nodiscard]]
double Ratio(const Nutrition& nutr, const std::string& unit,
             double value, Litre volume, Gram weight)
{
  if (unit == "ea" && nutr.wt < Gram::zero())
    return value;
  if (nutr.vol != Litre::zero()) {
    if (volume != Litre::zero())
      return value * double(volume / nutr.vol);
  }
  if (nutr.wt != Gram::zero()) {
    if (weight != Gram::zero())
      return value * double(weight / std::abs(nutr.wt));
  }
  return 0.0;
} // Ratio

struct Line {
  std::string value;
  std::string unit;
  std::string weight;
  std::string name;
  void erase() {
    value.erase();
    unit.erase();
    weight.erase();
    name.erase();
  }
}; // Line

[[nodiscard]]
auto MakeString(const Line& line) {
  auto rval = std::string{line.value};
  if (!line.unit.empty())
    rval += " " + line.unit;
  if (!line.weight.empty())
    rval += " (" + line.weight + ")";
  if (!line.name.empty())
    rval += " " + line.name;
  return rval;
} // MakeString(Line)

std::ostream& operator<<(std::ostream& os, const Line& line)
  { return os << MakeString(line); }

[[nodiscard]]
auto Parse(const std::string& str) -> Line {
  Line line;
  auto input = std::istringstream{str};
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

[[noreturn]]
void NewHandler() noexcept {
  std::set_new_handler(nullptr);
  std::cerr << "Out of memory!" << std::endl;
  std::terminate();
} // NewHandler

int main() {
  std::set_new_handler(NewHandler);
  try {
    NutrVec ingredients;
    ReadIngredients(ingredients);
    std::cout << "Read " << ingredients.size() << " ingredients." << std::endl;

    int servings = 0;
    Gram cookedWeight;
    Line line;
    std::string buf;
    Nutrition total;
    std::cin.exceptions(std::cin.failbit);
    while (std::cin) {
      std::cin >> std::ws;
      if (std::cin.eof())
	break;
      if (std::cin.peek() == '#') {
        static const auto all = std::numeric_limits<std::streamsize>::max();
	std::cin.ignore(all, '\n');
	continue;
      }
      std::getline(std::cin, buf);
      Line line = Parse(buf);
      { // Process servings specification.
	auto unit = ToLower(line.unit);
	if (unit == "serving" || unit == "servings") {
	  if (!line.name.empty() && line.name[0] != '#') {
	    throw std::runtime_error(
		"Invalid servings spec: " + MakeString(line));
	  }
	  if (servings != 0)
	    throw std::runtime_error("Duplicate servings: " + MakeString(line));
	  auto s = std::stod(line.value);
	  if (s < 1 || s > 100 || std::round(s) != s) {
	    throw std::runtime_error(
		"Invalid number of servings: " + MakeString(line));
	  }
	  Gram w;
	  if (!line.weight.empty()) {
	    std::istringstream iss(line.weight);
	    double v = 0.0;
	    std::string u;
	    iss >> v >> u;
	    w = v * FindWeight(FindUnit(u));
	    if (w <= Gram::zero()) {
	      throw std::runtime_error(
		  "Invalid serving weight: " + MakeString(line));
	    }
	  }
	  servings = gsl::narrow_cast<int>(s);
	  cookedWeight = w;
	  std::cout << "servings=" << servings;
	  if (cookedWeight != Gram::zero())
	    std::cout << ", cooked weight=" << std::ceil(cookedWeight.numerical_value_in(si::gram)) << " g";
	  std::cout << std::endl;
	  continue;
	}
      }
      auto value = Value(line.value);
      auto unit  = FindUnit(line.unit);
      Litre volume;
      Gram  weight;
      if (unit != "ea") {
        volume = FindVolume(unit);
	if (volume == Litre::zero()) {
	  weight = FindWeight(unit);
	  if (weight == Gram::zero() && line.weight.empty()) {
	    line.name = line.unit + ' ' + line.name;
	    unit = "ea";
	    line.unit.clear();
	  }
	}
      }
      std::optional<Nutrition> nutr;
      {
	auto name = ToLower(line.name);
	{ // trim punctuation
	  const auto punct = std::string("!$()*+:;<=>?@[]^{|}~");
	  auto i = name.find_first_of(punct);
	  if (i != std::string::npos)
	    name.erase(i);
	}
	TrimTrailingWs(name);
	if (!name.empty()) {
	  if (Contains(name, "extra")) {
	    static const std::regex
				e{"\\bextra[ -](small|large|light|heavy)\\b"};
	    name = std::regex_replace(name, e, "x$1");
	  }
	  nutr = FindIngredientWithPlurals(ingredients, name);
	  if (!nutr) {
	    // substitute common synonyms
	    rng::replace(name, '-', ' ');
	    static const std::regex e1{"\\b(diced|cubed)\\b"};
	    static const std::regex e2{"\\bdry\\b"};
	    static const std::regex e3{"\\bservings\\b"};
	    name = std::regex_replace(name, e1, "chopped");
	    name = std::regex_replace(name, e2, "dried");
	    name = std::regex_replace(name, e3, "serving");
	    nutr = FindIngredientWithPlurals(ingredients, name);
	  }
	}
      }
      if (!nutr)
	nutr = Nutrition();
      auto& nut = *nutr;
      nut.scale(Ratio(nut, unit, value, volume, weight));
      if (nut.wt != Gram::zero())
        nut.wt = std::max(std::abs(nut.wt), 0.1f * si::gram);
      using std::cout;
      using std::setw;
      using std::ceil;
      using std::round;
      {
        PrecSaver prec(cout, 1);
	cout << std::fixed
	  << "wt="     << setw(6) << nut.wt.numerical_value_in(si::gram)
	  << " kcal="  << setw(6) << nut.energy.numerical_value_in(si::Kcal)
	  << " p="     << setw(5) << nut.prot.numerical_value_in(si::gram)
	  << " f="     << setw(5) << nut.fat.numerical_value_in(si::gram)
	  << " c="     << setw(5) << nut.carb.numerical_value_in(si::gram)
	  << " fb="    << setw(5) << nut.fiber.numerical_value_in(si::gram)
	  << std::defaultfloat
	  << " : " << line.value;
      }
      if (!line.unit.empty())
	cout << ' ' << line.unit;
      if (!line.weight.empty()) {
	cout << " (";
        if (!std::isdigit(line.weight[0])) {
	  auto w = FindWeight(FindUnit(line.weight));
	  if (w == Gram::zero()) {
	    cout << line.weight << '?';
	  }
	  else {
	    PrecSaver prec(cout, 3);
	    cout << double(nut.wt / w) << ' ' << line.weight;
	  }
	}
	else {
	  cout << line.weight;
	  std::istringstream iss(line.weight);
	  double v = 0.0;
	  std::string u;
	  Gram wt;
	  iss >> v >> u;
	  if (iss)
	    wt = v * FindWeight(FindUnit(u));
	  if (wt <= Gram::zero() || (100 * std::abs(nut.wt-wt))/wt > 7) {
	    cout << '?';
	  }
	}
	cout << ')';
      }
      if (!line.name.empty())
	cout << ' ' << line.name;
      cout << std::endl;
      total += nut;
    }
    {
      using std::cout;
      using std::setw;
      using std::round;
      cout << '\n';
      if (servings != 0) {
	cout << "Per ";
	if (cookedWeight != 0.0 * si::gram)
	  cout << std::ceil(cookedWeight.numerical_value_in(si::gram)/servings) << " g ";
	cout << "serving:\n\n";
	total.scale(1.0/servings);
      }
      cout << setw(4) << round(total.energy.numerical_value_in(si::Kcal))     << " kcal\n"
	   << setw(4) << round(total.wt.numerical_value_in(si::gram))   << " g raw\n"
	   << setw(4) << round(total.prot.numerical_value_in(si::gram)) << " g protein\n"
	   << setw(4) << round(total.fat.numerical_value_in(si::gram))  << " g fat\n"
	   << setw(4) << round(total.carb.numerical_value_in(si::gram)) << " g carb\n"
	   << setw(4) << round(total.fiber.numerical_value_in(si::gram))<< " g fiber"
	   << std::endl;
    }
    return EXIT_SUCCESS;
  }
  catch (const std::ios_base::failure& fail) {
    std::cout << "ios_base::failure: " << fail.what() << '\n'
              << "    error code = " << fail.code().message() << std::endl;

  }
  catch (const std::exception& x) {
    std::cout << "standard exception: " << x.what() << std::endl;
  }
  return EXIT_FAILURE;
} // main
