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
#include <cstring>
#include <cctype>
#include <cstdlib>


namespace gsl {

template<class T, class U>
constexpr T narrow_cast(U&& u) noexcept {
  return static_cast<T>(std::forward<U>(u));
}

} // gsl

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

void TrimLeadingWs(std::string& str) {
  static const auto ws = std::string(" \t\n\r\f\v");
  str.erase(0, str.find_first_not_of(ws));
}

void TrimTrailingWs(std::string& str) {
  static const auto ws = std::string(" \t\n\r\f\v");
  auto i = str.find_last_not_of(ws);
  if (i != std::string::npos && ++i < str.size())
    str.erase(i);
}

bool ContainsAny(const std::string& str1, const std::string& str2)
{ return (str1.find_first_of(str2) != std::string::npos); }

bool Contains(const std::string& str, char ch)
{ return (str.find(ch) != std::string::npos); }

bool Contains(const char* str, char ch)
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

std::string SubstFraction(const std::string& str) {
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

double Value(const std::string& arg) {
  if (arg.empty())
    return 0;
  auto str = SubstFraction(arg);
  if (Contains(str, '.') || !ContainsAny(str, "-/ ")) {
    std::size_t pos = 0;
    double rval = 0.0;
    try { rval = std::stod(str, &pos); }
    catch (...) { return 0; }
    if (pos != str.size())
      return 0;
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

auto FindUnit(const std::string& unit) {
  if (unit.empty())
    return std::string("ea");
  auto u = ToLower(unit);
  auto it = UnitSyn.find(u);
  return (it != UnitSyn.end()) ? it->second : u;
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

auto Ratio(const Ingredient& ingred, const std::string& unit,
	   double value, double volume, double weight)
{
  if (unit == "ea" && ingred.g < 0.0)
    return value;
  if (ingred.ml != 0) {
    if (volume != 0.0)
      return value * volume / ingred.ml;
  }
  if (ingred.g != 0) {
    if (weight != 0.0)
      return value * weight / std::abs(ingred.g);
  }
  return 0.0;
} // Ratio

void Subst(std::string& str1, const std::string& str2, const std::string& str3)
{
  auto idx = str1.find(str2);
  if (idx != std::string::npos)
    str1.replace(idx, str2.size(), str3);
} // Subst

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

std::string MakeString(const Line& line) {
  std::string rval = line.value;
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
    const auto& ingredients = ReadIngredients();
    std::cout << "Read " << ingredients.size() << " ingredients." << std::endl;

    int servings = 0;
    double cookedWeight = 0.0;
    Line line;
    std::string buf;
    auto total = Ingredient("Total");
    std::cin.exceptions(std::cin.failbit);
    while (std::cin) {
      std::cin >> std::ws;
      if (std::cin.eof())
	break;
      if (std::cin.peek() == '#') {
	std::cin.ignore(1024, '\n');
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
	  double s = std::stod(line.value);
	  if (s < 1 || s > 100 || std::round(s) != s) {
	    throw std::runtime_error(
		"Invalid number of servings: " + MakeString(line));
	  }
	  double w = 0.0;
	  if (!line.weight.empty()) {
	    std::istringstream iss(line.weight);
	    double v = 0.0;
	    std::string u;
	    iss >> v >> u;
	    w = v * FindWeight(FindUnit(u));
	    if (w <= 0.0) {
	      throw std::runtime_error(
		  "Invalid serving weight: " + MakeString(line));
	    }
	  }
	  servings = gsl::narrow_cast<int>(s);
	  cookedWeight = w;
	  std::cout << "servings=" << servings;
	  if (cookedWeight)
	    std::cout << ", cooked weight=" << std::ceil(cookedWeight) << " g";
	  std::cout << std::endl;
	  continue;
	}
      }
      auto value = Value(line.value);
      auto unit  = FindUnit(line.unit);
      double volume = 0.0;
      double weight = 0.0;
      if (unit != "ea") {
        volume = FindVolume(unit);
	if (volume == 0.0) {
	  weight = FindWeight(unit);
	  if (weight == 0.0 && line.weight.empty()) {
	    line.name = line.unit + ' ' + line.name;
	    unit = "ea";
	    line.unit.clear();
	  }
	}
      }
      auto name = ToLower(line.name);
      { // trim punctuation
	const auto punct = std::string("!#$()*+,./:;<=>?@[]^{|}~");
	auto i = name.find_first_of(punct);
	if (i != std::string::npos)
	  name.erase(i);
      }
      TrimTrailingWs(name);
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
      ing.scale(Ratio(ing, unit, value, volume, weight));
      ing.g = std::abs(ing.g);
      using std::cout;
      using std::setw;
      using std::round;
      auto grams = gsl::narrow_cast<int>(round(ing.g));
      if (grams == 0 && ing.g != 0.0)
        grams = 1;
      cout
	<< "g="    << setw(3) << grams
	<< " cal=" << setw(4) << round(ing.cal)
	<< " p="   << setw(3) << round(ing.prot)
	<< " f="   << setw(3) << round(ing.fat)
	<< " c="   << setw(3) << round(ing.carb)
	<< " fb="  << setw(3) << round(ing.fiber)
	<< " : " << line.value;
      if (!line.unit.empty())
	cout << ' ' << line.unit;
      if (!line.weight.empty()) {
	cout << " (";
        if (!std::isdigit(line.weight[0])) {
	  double w = FindWeight(FindUnit(line.weight));
	  if (w == 0.0) {
	    cout << line.weight << '?';
	  }
	  else {
	    PrecSaver prec(cout, 3);
	    cout << (ing.g / w) << ' ' << line.weight;
	  }
	}
	else {
	  cout << line.weight;
	  std::istringstream iss(line.weight);
	  double v = 0.0;
	  std::string u;
	  int g = 0;
	  iss >> v >> u;
	  if (iss)
	    g = gsl::narrow_cast<int>(round(v * FindWeight(FindUnit(u))));
	  int z = gsl::narrow_cast<int>(round(ing.g));
	  if (g <= 0 || g != z)
	    cout << '?';
	}
	cout << ')';
      }
      if (!line.name.empty())
	cout << ' ' << line.name;
      cout << std::endl;
      total += ing;
    }
    {
      using std::cout;
      using std::setw;
      using std::round;
      cout << '\n';
      if (servings != 0) {
	cout << "Per ";
	if (cookedWeight != 0.0)
	  cout << std::ceil(cookedWeight/servings) << " g ";
	cout << "serving:\n\n";
	total.scale(1.0/servings);
      }
      cout << setw(4) << round(total.cal)  << " cal\n"
	   << setw(4) << round(total.g)    << " g raw\n"
	   << setw(4) << round(total.prot) << " g protein\n"
	   << setw(4) << round(total.fat)  << " g fat\n"
	   << setw(4) << round(total.carb) << " g carb\n"
	   << setw(4) << round(total.fiber)<< " g fiber"
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
