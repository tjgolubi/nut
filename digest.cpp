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
using VarItem = std::pair<std::regex, std::string>;
using VarMap = std::map<std::string, VarItem>;

#define COUT cout << fname << '(' << linenum << ") "

bool Contains(const std::string& str1, const auto& str2)
{ return (str1.find(str2) != std::string::npos); }

void ReadIngredients(const std::string& fname, NutritionMap& nuts, VarMap& defs)
{
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
  static const std::regex dollars_re("\\$\\$");
  Nutrition nutr;
  std::string key;
  std::string dollars;
  VarMap vars;
  auto subst_vars = [&](std::string& str) {
    if (!Contains(str, '$'))
      return;
    if (!dollars.empty())
      str = std::regex_replace(str, dollars_re, dollars);
    else if (Contains(str, "$$"))
      COUT << "$$ undefined\n";
    if (vars.empty())
      return;
    for (const auto& [var, val]: vars)
      str = std::regex_replace(str, val.first, val.second);
  }; // subst_vars
  bool allow_each = false;
  bool is_equal = false;
  bool ignore_flag = false;
  std::stack<bool> if_blocks;
  while (std::getline(input, line)) {
    ++linenum;

    // Remove comment and trailing whitespace.
    if (auto i = line.find("//"); i != npos)
      line.erase(i);
    if (auto i = line.find_last_not_of(ws); i != npos)
      line.erase(i + 1);

    if (line.empty())
      continue;

    if (std::regex_search(line, std::regex("^\\s*#"))) {
      line = std::regex_replace(line, std::regex("^\\s*#\\s*"), "");
      if (line == "endif") {
        if (if_blocks.empty()) {
	  COUT << "unmatched #endif\n";
	  continue;
	}
        ignore_flag = if_blocks.top();
	if_blocks.pop();
	continue;
      }

      if (line.starts_with("if"))
        if_blocks.push(ignore_flag);

      if (ignore_flag)
	continue;

      if (line == "if 0") {
	ignore_flag = true;
	continue;
      }

      std::smatch s;
      if (line.starts_with("ifdef")) {
        static const std::regex e{"ifdef\\s+(\\w+)"};
	if (!std::regex_match(line, s, e)) {
	  COUT << "invalid #ifdef\n";
	  continue;
	}
	ignore_flag = !defs.contains(s[1].str());
	continue;
      }

      if (line.starts_with("ifndef")) {
        static const std::regex e{"ifndef\\s+(\\w+)"};
	if (!std::regex_match(line, s, e)) {
	  COUT << "invalid #ifndef\n";
	  continue;
	}
	ignore_flag = defs.contains(s[1].str());
	continue;
      }

      if (line.starts_with("include")) {
	static const std::regex e{"include\\s*\"([^\"]+)\""};
	if (!std::regex_match(line, s, e)) {
	  COUT << "invalid #include\n";
	  continue;
	}
	ReadIngredients(s[1].str(), nuts, defs);
	continue;
      }
      if (line.starts_with("define")) {
        static const std::regex e1{"define\\s+(\\w+)"};
        static const std::regex e2{"define\\s+(\\w+)\\s+(.*)"};
	std::string var;
	std::string val;
	if (std::regex_match(line, s, e1)) {
	  var = s[1].str();
	}
	else if (std::regex_match(line, s, e2)) {
	  var = s[1].str();
	  val = s[2].str();
	  for (const auto& [dvar, dval]: defs)
	    val = std::regex_replace(val, dval.first, dval.second);
	}
	else {
	  COUT << "invalid #define\n";
	  continue;
	}

	auto iter = defs.find(var);
	if (iter == defs.end()) {
	  VarItem v{std::regex{"\\b" + var + "\\b"}, val};
	  defs.emplace(var, std::move(v));
	  continue;
	}

        auto& oldval = iter->second.second;
	if (val != oldval) {
	  COUT "redefining " << var << ": "
	    << std::quoted(oldval) << " --> " << std::quoted(val) << '\n';
	}
	oldval = val;
	continue;
      }
      if (line.starts_with("undef")) {
        static const std::regex e{"undef\\s+(\\w+)"};
	if (!std::regex_match(line, s, e)) {
	  COUT << "invalid #undef\n";
	  continue;
	}
	defs.erase(s[1].str());
	continue;
      }
      continue;
    }

    if (ignore_flag)
      continue;

    for (const auto& [var, val]: defs)
      line = std::regex_replace(line, val.first, val.second);

    // std::cout << line << '\n';

    std::istringstream istr{line};

    istr >> std::ws;
    if (!istr)
      continue;

    if (istr.peek() == ':') {
      istr.ignore();
      istr >> std::ws;
      if (istr.eof()) {
	dollars.clear();
	vars.clear();
	continue;
      }
      if (!Contains(line, '=')) {
	dollars.clear();
	std::getline(istr, dollars);
	continue;
      }
      static const auto e = std::regex{"\\s*:\\s*(\\w+)\\s*=\\s*(.*)"};
      std::smatch m;
      if (!std::regex_match(line, m, e)) {
	COUT << "invalid variable definition\n";
	continue;
      }
      auto var = m[1].str();
      auto val = m[2].str();
      if (val.empty()) {
	vars.erase(var);
	continue;
      }
      subst_vars(val);
      vars[var] = VarItem("\\$" + var + "\\b", val);
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
      subst_vars(key);
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

    subst_vars(name);

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
      subst_vars(key);
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

      double scale = 0.0;
      if (nutr.kcal != 0.0)
	scale = nutr.kcal / n.kcal;
      else if (nutr.g != 0.0 && n.g != 0.0)
	scale = nutr.g / n.g;
      else if (nutr.ml != 0.0 && n.ml != 0.0)
	scale = nutr.ml / n.ml;
      if (scale == 0.0) {
	COUT << "0 scale: " << std::quoted(key) << '\n';
	continue;
      }
      nutr.prot  = scale * n.prot;
      nutr.fat   = scale * n.fat;
      nutr.carb  = scale * n.carb;
      nutr.fiber = scale * n.fiber;
      if (nutr.kcal == 0.0)
	nutr.kcal = scale * n.kcal;
      if (nutr.g == 0.0)
	nutr.g = scale * std::abs(n.g);
      if (nutr.ml == 0.0)
	nutr.ml = scale * n.ml;
    }

    if (nutr.g == 0.0) {
      if (nutr.ml == 0.0 && !allow_each)
	COUT << "allow each assumed\n";
    }
    else {
      nutr.g = allow_each ? -std::abs(nutr.g) : std::abs(nutr.g);
    }
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
    VarMap defs;
    ReadIngredients("ingred.txt", ingredients, defs);
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
