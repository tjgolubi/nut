// Copyright 2023 Terry Golubiewski, all rights reserved.

#include "Nutrition.h"

#include <gsl/gsl>

#include <string>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <regex>
#include <ranges>
#include <algorithm>

namespace rng = std::ranges;

using NutritionMap = std::map<std::string, Nutrition>;

#define COUT cout << fname << '(' << linenum << ") "

void ReadIngredients(const std::string& fname, NutritionMap& nuts) {
  using std::cout;
  auto input = std::ifstream(fname);
  if (!input || !input.is_open()) {
    cout << fname << ": could not read\n";
    return;
  }
  std::string line;
  int linenum = 0;
  std::string name;
  static const auto npos = std::string::npos;
  static const auto ws   = " \t\n\r\f\v";
  static const std::regex company_re("\\$\\$");
  Nutrition nutr;
  std::string key;
  std::string company;
  bool allow_each = false;
  bool is_equal = false;
  while (std::getline(input, line)) {
    ++linenum;

    // Remove comment and trailing whitespace.
    if (auto i = line.find("//"); i != npos)
      line.erase(i);
    if (auto i = line.find_last_not_of(ws); i != npos)
      line.erase(i + 1);

    if (line.empty())
      continue;

    std::istringstream istr{line};

    istr >> std::ws;
    if (!istr)
      continue;

    if (istr.peek() == '#') {
      std::smatch s;
      static const std::regex e{" *# *include *\"([^\"]+)\""};
      if (!std::regex_match(line, s, e) || !s.ready() || s.size() != 2) {
        if (line.find("include") != npos)
          COUT << "invalid #include\n";
	continue;
      }
      const auto& incl = s[1].str();
      COUT << "#include " << std::quoted(incl) << '\n';
      ReadIngredients(incl, nuts);
      continue;
    }

    if (istr.peek() == ':') {
      istr.ignore();
      istr >> std::ws >> company;
      continue;
    }

    allow_each = (istr.peek() == '*');
    if (allow_each) {
      istr.ignore();
      istr >> std::ws;
    }

    nutr.zero();
    is_equal = (istr.peek() == '=');
    if (is_equal) {
      istr.ignore();
      istr >> std::ws >> std::quoted(key);
      if (!istr) {
	COUT << " invalid key\n";
	continue;
      }
      if (!company.empty())
        key = std::regex_replace(key, company_re, company);
      auto iter = nuts.find(key);
      if (iter == nuts.end()) {
	COUT << "key not found: " << std::quoted(key) << '\n';
	continue;
      }
      nutr = iter->second;
      key.clear();
    }
    else {
      istr >> nutr.g >> nutr.ml >> nutr.kcal >> std::ws;;
      if (!istr) {
	COUT << "invalid nutrition\n";
	continue;
      }
      key.clear();
      if (auto c = istr.peek(); std::isdigit(c) || c == '.')
	istr >> nutr.prot >> nutr.fat >> nutr.carb >> nutr.fiber;
      else
	istr >> std::quoted(key);
      if (!istr) {
	COUT << "invalid macros\n";
	continue;
      }
    }
    std::getline(istr >> std::ws, name);
    if (!istr || name.empty()) {
      COUT << "invalid name\n";
      continue;
    }

    if (!company.empty())
      name = std::regex_replace(name, company_re, company);

    static auto is_upper = [](unsigned char c) -> bool
      { return std::isupper(c); };
    static auto to_lower = [](unsigned char c) -> unsigned char
      { return std::tolower(c); };
    if (auto iter = rng::find_if(name, is_upper); iter != name.end()) {
      COUT << "upper-case name converted\n";
      rng::transform(iter, name.end(), iter, to_lower);
    }

#if 0
    if (!is_equal && key.empty()) {
      auto kcal = 4 * (nutr.prot + nutr.carb - nutr.fiber) + 9 * nutr.fat;
      auto err = std::abs(kcal - nutr.kcal);
      if (nutr.kcal != 0.0)
        err /= nutr.kcal;
      if (std::abs(err) > 0.1) {
	cout << fname << ':' << linenum << ": kcal warning: "
	    << kcal << " != " << nutr << ' ' << name << '\n';
      }
    }
#endif

    if (!key.empty()) {
      if (!company.empty())
        key = std::regex_replace(key, company_re, company);
      auto iter = nuts.find(key);
      if (iter == nuts.end()) {
	COUT << "key not found: " << std::quoted(key) << '\n';
	continue;
      }
      const auto& n = iter->second;
      if (n.kcal == 0.0) {
        COUT << "zero base kcal\n";
	continue;
      }
      const auto scale = nutr.kcal / n.kcal;
      nutr.prot  = scale * n.prot;
      nutr.fat   = scale * n.fat;
      nutr.carb  = scale * n.carb;
      nutr.fiber = scale * n.fiber;
      if (nutr.g == 0.0)
	nutr.g = scale * std::abs(n.g);
      if (nutr.ml == 0.0)
	nutr.ml = scale * n.ml;
    }

    if (nutr.g == 0.0 && nutr.ml == 0.0 && !allow_each)
      COUT << "allow each assumed\n";

    if (allow_each && nutr.g > 0.0)
      nutr.g = -nutr.g;
#if 0
      cout << std::left << std::setw(40) << std::quoted(name) << ' '
		<< std::right << nutr << '\n';
#endif
    if (!nuts.try_emplace(name, nutr).second)
      COUT << "duplicate: " << name << '\n';
  }
} // ReadIngredients

int main() {
  using std::cout;
  try {
    NutritionMap ingredients;
    ReadIngredients("ingred.txt", ingredients);
    cout << "Read " << ingredients.size() << " ingredients." << std::endl;

    std::ofstream output{"ingred.dat", std::ios::binary};

    for (const auto& [name, nutr]: ingredients) {
      output.write(name.c_str(), name.size()+1);
      output.write(reinterpret_cast<const char*>(&nutr), sizeof(nutr));
    }

    return EXIT_SUCCESS;
  }
  catch (const std::ios::failure& fail) {
    cout << "ios::failure: " << fail.what()
              << "\n    error code = " << fail.code().message() << std::endl;

  }
  catch (const std::exception& x) {
    cout << "standard exception: " << x.what() << std::endl;
  }

  return EXIT_FAILURE;
} // main
