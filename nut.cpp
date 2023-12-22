// Copyright 2023 Terry Golubiewski, all rights reserved.

#include <gsl/gsl>

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

struct Nutrition {
  double g     = 0.0;
  double ml    = 0.0;
  double kcal  = 0.0;
  double prot  = 0.0;
  double fat   = 0.0;
  double carb  = 0.0;
  double fiber = 0.0;
  void zero() {
    g     = 0.0;
    ml    = 0.0;
    kcal  = 0.0;
    prot  = 0.0;
    fat   = 0.0;
    carb  = 0.0;
    fiber = 0.0;
  }
  void scaleMacros(double ratio) {
    prot  *= ratio;
    fat   *= ratio;
    carb  *= ratio;
    fiber *= ratio;
  }
  void scale(double ratio) {
    g     *= ratio;
    ml    *= ratio;
    kcal  *= ratio;
    scaleMacros(ratio);
  }
  Nutrition& operator+=(const Nutrition& rhs) {
    g     += rhs.g;
    ml    += rhs.ml;
    kcal  += rhs.kcal;
    prot  += rhs.prot;
    fat   += rhs.fat;
    carb  += rhs.carb;
    fiber += rhs.fiber;
    return *this;
  }
  bool operator==(const Nutrition& rhs) const = default;
  auto operator<=>(const Nutrition& lhs) const = default;
}; // Nutrition

std::ostream& operator<<(std::ostream& output, const Nutrition& nutr) {
  using std::setw;
  using std::round;
  PrecSaver precSaver(output, 1);
  output << std::fixed
         << ' ' << setw(7) << round(nutr.g)
	 << ' ' << setw(6) << round(nutr.ml)
	 << ' ' << setw(6) << round(nutr.kcal)
	 << ' ' << setw(5) << nutr.prot
	 << ' ' << setw(5) << nutr.fat
	 << ' ' << setw(5) << nutr.carb
	 << ' ' << setw(5) << nutr.fiber
	 << std::defaultfloat;
  return output;
} // << Nutrition

std::istream& operator>>(std::istream& input, Nutrition& nutr) {
  nutr = Nutrition();
  return input >> nutr.g >> nutr.ml >> nutr.kcal
	>> nutr.prot >> nutr.fat >> nutr.carb >> nutr.fiber;
}

using NutrMap = std::map<std::string, Nutrition>;

#if 0
auto Compare(const Ingredient& ingred, const std::string& name)
{ return (ingred.name <=> name); }
#endif

auto FindIngredient(const NutrMap& ingredients,
                    const std::string& name)
  -> std::optional<Nutrition>
{
  if (name.empty())
    return std::nullopt;

  auto r = ingredients.find(name);
  if (r == ingredients.end())
    return std::nullopt;

  return r->second;
} // FindIngredient

auto FindIngredientWithPlurals(const NutrMap& ingredients,
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

auto ReadIngredients() {
  std::string str;
  using NutrMap = std::map<std::string, Nutrition>;
  NutrMap rval;
  auto input = std::ifstream("ingred.txt");
  if (!input)
    return rval;
  std::string name;
  Nutrition nutr;
  std::string key;
  bool allow_each = false;
  bool is_equal = false;
  static const auto all = std::numeric_limits<std::streamsize>::max();
  while (input >> std::ws) {
    if (input.peek() == '#') {
      input.ignore(all, '\n');
      continue;
    }
    if (input.peek() == '/') {
      input.ignore();
      if (input.peek() == '/') {
        input.ignore(all, '\n');
	continue;
      }
      input.unget();
    }
    allow_each = (input.peek() == '*');
    if (allow_each) {
      input.ignore();
      input >> std::ws;
    }
    nutr.zero();
    is_equal = (input.peek() == '=');
    if (is_equal) {
      input.ignore();
      input >> std::ws >> std::quoted(key);
      auto iter = rval.find(key);
      if (iter != rval.end())
        nutr = iter->second;
      else
	std::cout << std::quoted(key) << " not found.\n";
      key.clear();
    }
    else {
      input >> nutr.g >> nutr.ml >> nutr.kcal >> std::ws;
      key.clear();
      if (auto c = input.peek(); std::isdigit(c) || c == '.')
	input >> nutr.prot >> nutr.fat >> nutr.carb >> nutr.fiber;
      else
	input >> std::quoted(key);
    }
    std::getline(input >> std::ws, name);
    if (auto i = name.rfind("//"); i != std::string::npos) {
      name.erase(i);
      TrimTrailingWs(name);
    }
#if 0
    if (!is_equal && key.empty()) {
      auto kcal = 4 * (nutr.prot + nutr.carb - nutr.fiber) + 9 * nutr.fat;
      auto err = std::abs(kcal - nutr.kcal);
      if (nutr.kcal != 0.0)
        err /= nutr.kcal;
      if (std::abs(err) > 0.1)
	std::cout << "kcal warning: " << kcal << " != " << nutr << ' ' << name
	          << '\n';
    }
#endif
    if (!key.empty()) {
      auto iter = rval.find(key);
      if (iter != rval.end()) {
	const Nutrition& n = iter->second;
	const double scale = double(nutr.kcal) / n.kcal;
	nutr.prot  = scale * n.prot;
	nutr.fat   = scale * n.fat;
	nutr.carb  = scale * n.carb;
	nutr.fiber = scale * n.fiber;
	if (nutr.g == 0.0)
	  nutr.g = scale * std::abs(n.g);
	if (nutr.ml == 0.0)
	  nutr.ml = scale * n.ml;
      }
      else {
	std::cout << std::quoted(key) << " not found " << name << '\n';
	nutr.zero();
      }
    }
    if (!name.empty() && (nutr.g != 0.0 || nutr.ml != 0.0)) {
      if (allow_each && nutr.g > 0.0)
	nutr.g = -nutr.g;
#if 0
      std::cout << std::left << std::setw(40) << std::quoted(name) << ' '
		<< std::right << nutr << '\n';
#endif
      if (!rval.try_emplace(name, nutr).second)
        std::cout << "Duplicate: " << name << '\n';
    }
  }
  return rval;
} // ReadIngredients

bool ContainsAny(const std::string& str1, const std::string& str2)
{ return (str1.find_first_of(str2) != std::string::npos); }

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

auto Ratio(const Nutrition& nutr, const std::string& unit,
	   double value, double volume, double weight)
{
  if (unit == "ea" && nutr.g < 0.0)
    return value;
  if (nutr.ml != 0) {
    if (volume != 0.0)
      return value * volume / nutr.ml;
  }
  if (nutr.g != 0) {
    if (weight != 0.0)
      return value * weight / std::abs(nutr.g);
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
      std::optional<Nutrition> nutr;
      {
	auto name = ToLower(line.name);
	{ // trim punctuation
	  const auto punct = std::string("!#$()*+,./:;<=>?@[]^{|}~");
	  auto i = name.find_first_of(punct);
	  if (i != std::string::npos)
	    name.erase(i);
	}
	TrimTrailingWs(name);
	if (!name.empty()) {
	  Subst(name, "extra small", "xsmall");
	  Subst(name, "extra large", "xlarge");
	  nutr = FindIngredientWithPlurals(ingredients, name);
	  if (!nutr) {
	    // substitute common synonyms
	    Subst(name, "diced", "chopped");
	    Subst(name, "cubed", "chopped");
	    Subst(name, "dry",   "dried");
	    nutr = FindIngredientWithPlurals(ingredients, name);
	  }
	}
      }
      if (!nutr)
	nutr = Nutrition();
      auto& nut = *nutr;
      nut.scale(Ratio(nut, unit, value, volume, weight));
      if (nut.g != 0.0)
        nut.g = std::max(std::abs(nut.g), 0.1);
      using std::cout;
      using std::setw;
      using std::ceil;
      using std::round;
      {
        PrecSaver prec(cout, 1);
	cout << std::fixed
	  << "g="     << setw(6) << nut.g
	  << " kcal=" << setw(6) << nut.kcal
	  << " p="    << setw(5) << nut.prot
	  << " f="    << setw(5) << nut.fat
	  << " c="    << setw(5) << nut.carb
	  << " fb="   << setw(5) << nut.fiber
	  << std::defaultfloat
	  << " : " << line.value;
      }
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
	    cout << (nut.g / w) << ' ' << line.weight;
	  }
	}
	else {
	  cout << line.weight;
	  std::istringstream iss(line.weight);
	  double v = 0.0;
	  std::string u;
	  double g = 0.0;
	  iss >> v >> u;
	  if (iss)
	    g = v * FindWeight(FindUnit(u));
	  if (g <= 0.0 || (100 * std::abs(nut.g-g))/g > 7) {
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
	if (cookedWeight != 0.0)
	  cout << std::ceil(cookedWeight/servings) << " g ";
	cout << "serving:\n\n";
	total.scale(1.0/servings);
      }
      cout << setw(4) << round(total.kcal) << " kcal\n"
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
