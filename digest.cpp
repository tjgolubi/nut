// Copyright 2023 Terry Golubiewski, all rights reserved.

#include <gsl/gsl>

#include <ranges>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <cmath>
#include <cstring>
#include <cctype>
#include <cstdlib>

#include <exception>

class FixPt {
  using storage_type = std::int32_t;
  storage_type mVal = storage_type{};
  static constexpr auto Scale = storage_type{1} << 16;

public:
  FixPt() : mVal{} { }
  explicit FixPt(double x) : mVal{gsl::narrow<storage_type>(std::floor(x * Scale))} { } 
  operator double() const { return (double(mVal) + 0.5) / Scale; }
  std::ostream& write(std::ostream& os) const
  { return os.write(reinterpret_cast<const char*>(&mVal), sizeof(mVal)); }
  std::istream& read(std::istream& is)
  { return is.read(reinterpret_cast<char*>(&mVal), sizeof(mVal)); }
  friend std::ostream& write(std::ostream& os, const FixPt& x) { return x.write(os); }
  friend std::istream& read(std::istream& is, FixPt& x) { return x.read(is); }
}; // FixPt

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
  std::ostream& write(std::ostream& os) const {
    FixPt{g}    .write(os);
    FixPt{ml}   .write(os);
    FixPt{kcal} .write(os);
    FixPt{prot} .write(os);
    FixPt{fat}  .write(os);
    FixPt{carb} .write(os);
    FixPt{fiber}.write(os);
    return os;
  }
  std::istream& read(std::istream& is) {
    FixPt x;
    x.read(is); g     = x;
    x.read(is); ml    = x;
    x.read(is); kcal  = x;
    x.read(is); prot  = x;
    x.read(is); fat   = x;
    x.read(is); carb  = x;
    x.read(is); fiber = x;
    return is;
  }
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

auto ReadIngredients(const std::filesystem::path& fname, NutrMap& nuts) -> NutrMap& {
  auto input = std::ifstream(fname);
  if (!input)
    return nuts;
  std::string name;
  Nutrition nutr;
  std::string key;
  bool allow_each = false;
  bool is_equal = false;
  constexpr auto all = std::numeric_limits<std::streamsize>::max();
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
      auto iter = nuts.find(key);
      if (iter != nuts.end())
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
      auto iter = nuts.find(key);
      if (iter != nuts.end()) {
	const auto& n = iter->second;
	const auto scale = double(nutr.kcal) / n.kcal;
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
      if (!nuts.try_emplace(name, nutr).second)
        std::cout << "Duplicate: " << name << '\n';
    }
  }
  return nuts;
} // ReadIngredients

int main() {
  try {
    NutrMap ingredients;
    ReadIngredients(std::filesystem::path{"ingred.txt"}, ingredients);
    std::cout << "Read " << ingredients.size() << " ingredients." << std::endl;

    {
    auto output = std::ofstream{"ingred.dat", std::ios::binary};
    auto debug  = std::ofstream{"out.txt"};

    for (const auto& [name, nutr]: ingredients) {
      debug << nutr << ' ' << name << '\n';
      output.write(name.c_str(), name.size()+1);
      nutr.write(output);
    }
    }

  {
    auto input = std::ifstream{"ingred.dat", std::ios::binary};
    std::string name;
    Nutrition nutr;
    NutrMap ingred;
    while (std::getline(input, name, '\0')) {
      nutr.read(input);
      ingred.emplace(name, nutr);
    }
    std::cout << "Read " << ingred.size() << " ingredients.\n";
    if (ingredients == ingred)
      std::cout << "Match!" << std::endl;
    else
      std::cout << "No match." << std::endl;

    auto tjg = std::ofstream("tjg.txt");
    for (const auto& [x, y]: ingred)
      tjg << y << ' ' << x << '\n';
  }

    return EXIT_SUCCESS;
  }
  catch (const std::ios::failure& fail) {
    std::cout << "ios::failure: " << fail.what()
              << "\n    error code = " << fail.code().message() << std::endl;

  }
  catch (const std::exception& x) {
    std::cout << "standard exception: " << x.what() << std::endl;
  }
  return EXIT_FAILURE;
} // main
