// Copyright 2023-2025 Terry Golubiewski, all rights reserved.

#include "Nutrition.h"
#include "Atwater.h"

#include <mp-units/ostream.h>

#include <gsl/gsl>

#include <string>
#include <map>
#include <stack>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <regex>
#include <ranges>
#include <algorithm>
#include <cmath>

namespace rng = std::ranges;

using NutritionMap = std::map<std::string, Nutrition>;

namespace std {

template<auto T, typename R>
inline constexpr auto abs(::mp_units::quantity<T, R> x) {
  return (x >= ::mp_units::quantity<T, R>::zero())
         ?  x : -x;
}

template<auto T, typename R>
auto to_string(const ::mp_units::quantity<T, R>& x) {
  ostringstream os;
  os << x;
  return os.str();
}

} // std

struct VarItem {
  std::regex  re;
  std::string str;
}; // VarItem

using VarMap = std::map<std::string, VarItem>;

#define COUT cout << fname << '(' << linenum << ") "

bool Contains(const std::string& str1, const auto& str2)
{ return (str1.find(str2) != std::string::npos); }

void ReadIngredients(const std::string& fname, NutritionMap& nuts, VarMap& defs)
{
  using std::cout;
  using namespace mp_units;
  using namespace mp_units::si;

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
      str = std::regex_replace(str, val.re, val.str);
  }; // subst_vars
  bool allow_each = false;
  bool is_equal = false;
  bool kcal_range_error = false;
  bool ignore_flag = false;
  struct IfBlock {
    bool was_ignore = false;
    bool else_flag  = false;
    explicit IfBlock(bool ignore) : was_ignore(ignore) { }
  }; // IfBlock
  std::stack<IfBlock> if_blocks;
  Atwater atwater;
  std::string this_name;
  Nutrition this_nutr;
  while (std::getline(input, line)) {
    ++linenum;

    try {

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
	  ignore_flag = if_blocks.top().was_ignore;
	  if_blocks.pop();
	  continue;
	}

	if (line == "else") {
	  if (if_blocks.empty()) {
	    COUT << "unmatched #else\n";
	    ignore_flag = true;
	    continue;
	  }
	  if (if_blocks.top().else_flag) {
	    COUT << "unexpected #else\n";
	    ignore_flag = true;
	    continue;
	  }
	  auto& top = if_blocks.top();
	  ignore_flag = ignore_flag ? top.was_ignore : true;
	  top.else_flag = true;
	  continue;
	}

	if (line.starts_with("if"))
	  if_blocks.push(IfBlock(ignore_flag));

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

	if (line.starts_with("if")) {
	  COUT << "invalid #if\n";
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
	      val = std::regex_replace(val, dval.re, dval.str);
	  }
	  else {
	    COUT << "invalid #define\n";
	    continue;
	  }

	  auto iter = defs.find(var);
	  if (iter == defs.end()) {
	    defs.emplace(var, VarItem{std::regex{"\\b" + var + "\\b"}, val});
	    continue;
	  }

	  auto& oldval = iter->second.str;
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
	if (line.starts_with("echo")) {
	  auto i = line.find_first_not_of(ws, 4);
	  if (i == 4) {
	    COUT << "invalid #echo\n";
	    continue;
	  }
	  auto str = line.substr(i);
	  for (const auto& [dvar, dval]: defs)
	    str = std::regex_replace(str, dval.re, dval.str);
	  COUT << str << '\n';
	  continue;
	}
	continue;
      }

      if (ignore_flag)
	continue;

      for (const auto& [var, val]: defs)
	line = std::regex_replace(line, val.re, val.str);

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
	vars[var] = VarItem{std::regex{"\\$" + var + "\\b"}, val};
	continue;
      }

      if (istr.peek() == '[') {
	istr >> atwater;
	if (!istr)
	  COUT << "invalid Atwater factors\n";
	continue;
      }

      if (istr.peek() == '/') {
	static const std::regex e{"^\\s*/([^/]*)/([^/]*)/$"};
	subst_vars(line);
	std::smatch m;
	if (!std::regex_match(line, m, e)) {
	  COUT << "invalid search and replace\n";
	  continue;
	}
	// std::regex s{"\\b" + m[1].str() + "\\b"};
	std::regex s{m[1].str()};
	const auto& r = m[2].str();
	std::vector<NutritionMap::value_type> add;
	for (const auto& n: nuts) {
	  if (std::regex_search(n.first, s))
	    add.emplace_back(std::regex_replace(n.first, s, r), n.second);
	}
	for (auto& v: add)
	  nuts.emplace(std::move(v));
  #if 0
	COUT << std::quoted(m[1].str()) << " --> " << std::quoted(m[2].str())
	     << " " << add.size() << " times\n";
  #endif
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
	if (key == "this") {
	  nutr = this_nutr;
	}
	else {
	  auto iter = nuts.find(key);
	  if (iter == nuts.end()) {
	    COUT << "key not found: " << std::quoted(key) << '\n';
	    continue;
	  }
	  nutr = iter->second;
	}
	key.clear();
      }
      else {
        auto wt  = 0.0f;
        auto vol = 0.0f;
	istr >> wt >> vol >> std::ws;
	if (!istr) {
	  COUT << "invalid nutrition\n";
	  continue;
	}
        nutr.wt  = wt  * gram;
        nutr.vol = vol * milli<litre>;
	if (istr.peek() == '=') {
	  istr.ignore();
	  if (nutr.wt == 0.0f * gram && nutr.vol == 0.0f * milli<litre>) {
	    COUT << "Equivalence must specify either weight or volume\n";
	    continue;
	  }
	  nutr.energy = -1.0f * Calorie;
	}
	else {
          auto energy = 0.0f;
	  istr >> energy;
	  if (!istr) {
	    COUT << "invalid nutrition\n";
	    continue;
	  }
          nutr.energy = energy * Kcal;
	}
	kcal_range_error = (istr.peek() == '?');
	if (kcal_range_error)
	  istr.ignore();
	istr >> std::ws;
	key.clear();
	if (auto c = istr.peek(); std::isdigit(c) || c == '.') {
          auto prot  = 0.0f;
          auto fat   = 0.0f;
          auto carb  = 0.0f;
          auto fiber = 0.0f;
	  istr >> prot >> fat >> carb >> fiber;
          if (!istr) {
	    COUT << "invalid nutrition\n";
	    continue;
          }
          nutr.prot = prot * gram;
          nutr.fat  = fat  * gram;
          nutr.carb = carb * gram;
	  if (fiber < 0.0f) {
	    nutr.alcohol = -fiber * gram;
	    nutr.fiber = 0.0f * gram;
	  }
	  else if (nutr.carb < nutr.fiber)
	    throw std::runtime_error{"Invalid carbs: " + std::to_string(nutr.carb) + " < " + std::to_string(nutr.fiber)};
	}
	else {
	  istr >> std::quoted(key);
	}
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

      if (!is_equal && key.empty()) {
	using std::abs;
	using std::round;
	auto energy = atwater.energy(nutr);
	if (nutr.energy < 0.0f * Calorie) {
	  nutr.energy = energy;
	}
	else {
	  auto err = double(abs(energy - nutr.energy) / nutr.energy);
          auto energy_kcal = round(energy.numerical_value_in(Kcal));
          auto nutr_kcal   = round(nutr.energy.numerical_value_in(Kcal));
	  bool error = (abs(err) > 0.11 && abs(energy_kcal - nutr_kcal) > 1);
	  if (!error && kcal_range_error) {
	    COUT << "? not needed\n";
	  }
	  else if (error && !kcal_range_error) {
	    COUT << "kcal warning: "
		 << nutr_kcal << " != " << energy_kcal
		 << ' ' << name << '\n';
	    COUT << '[' << atwater.values_str() << "] "
		 << nutr << '\n';
	  }
	}

	vars["this"] = VarItem{std::regex{"\\$this\\b"}, name};
	this_name = name;
	this_nutr = nutr;
      }

      if (!key.empty()) {
	subst_vars(key);

	Nutrition* nptr = nullptr;;
	if (key == "this") {
	  nptr = &this_nutr;
	}
	else {
	  auto iter = nuts.find(key);
	  if (iter == nuts.end()) {
	    COUT << "key not found: " << std::quoted(key) << '\n';
	    continue;
	  }
	  nptr = &iter->second;
	}
	auto const& n = *nptr;

	if (n.energy == 0.0 * Kcal) {
	  COUT << "zero base kcal\n";
	  continue;
	}

        if (nutr.energy >= 0.0f * Kcal) {
	  double scale = 0.0;
	  if (nutr.energy != 0.0 * Kcal)
	    scale = double(nutr.energy / n.energy);
	  else if (nutr.wt != 0.0 * gram && n.wt != 0.0 * gram)
	    scale = double(nutr.wt / std::abs(n.wt));
	  else if (nutr.vol != 0.0 * litre && n.vol != 0.0 * litre)
	    scale = double(nutr.vol / n.vol);
	  if (scale == 0.0) {
	    COUT << "0 scale: " << std::quoted(key) << '\n';
	    continue;
	  }
	  nutr.prot    = scale * n.prot;
	  nutr.fat     = scale * n.fat;
	  nutr.carb    = scale * n.carb;
	  nutr.fiber   = scale * n.fiber;
	  nutr.alcohol = scale * n.alcohol;
	  if (nutr.energy == 0.0 * Kcal)
	    nutr.energy = scale * n.energy;
	  if (nutr.wt == 0.0 * gram)
	    nutr.wt = scale * std::abs(n.wt);
	  if (nutr.vol == nutr.vol.zero())
	    nutr.vol = scale * n.vol;
	}
	else {
	  nutr.energy = n.energy;
	  nutr.prot = n.prot;
	  nutr.fat  = n.fat;
	  nutr.carb = n.carb;
	  nutr.fiber = n.fiber;
	  nutr.alcohol = n.alcohol;
	  if (nutr.wt == 0.0 * gram)
	    nutr.wt = std::abs(n.wt);
	  if (nutr.vol == 0.0 * litre)
	    nutr.vol = n.vol;
	}
      }

      if (nutr.wt == 0.0 * gram) {
	if (nutr.vol == 0.0 * litre && !allow_each)
	  COUT << "allow each assumed\n";
      }
      else {
	nutr.wt = allow_each ? -std::abs(nutr.wt) : std::abs(nutr.wt);
      }
  #if 0
	cout << std::left << std::setw(40) << std::quoted(name) << ' '
		  << std::right << nutr << '\n';
  #endif

      if (name == "replace") {
	nuts[this_name] = this_nutr = nutr;
	continue;
      }

      { // detect punctuation
	const auto punct = "!$()*+:;<=>?@[]^{|}~";
	if (auto i = name.find_first_of(punct); i != std::string::npos) {
	  COUT << "Invalid punctuation erased: " << name.substr(i) << '\n';
	  name.erase(i);
	}
      }

      if (name.empty()) {
	COUT << "Empty name ignored";
	continue;
      }

      if (Contains(name, "extra")) {
	static const std::regex
			    e{"\\bextra[ -](small|large|light|heavy)\\b"};
	name = std::regex_replace(name, e, "x$1");
      }

      // substitute common synonyms
      {
	static const std::regex e1{"\\b(diced|cubed)\\b"};
	static const std::regex e3{"\\bservings\\b"};
	name = std::regex_replace(name, e1, "chopped");
	name = std::regex_replace(name, e3, "serving");
      }

      if (!nuts.try_emplace(name, nutr).second)
	COUT << "duplicate: " << name << '\n';
    }
    catch (const std::exception& x) {
      COUT << "Exception: " << x.what() << '\n';
    }
  }
} // ReadIngredients

int main(int argc, char* argv[]) {
  using std::cout;
  try {
    std::string input_file = (argc == 1) ? "ingred.nut" :  argv[1];
    NutritionMap ingredients;
    VarMap defs;
    ReadIngredients(input_file, ingredients, defs);
    cout << "Read " << ingredients.size() << " ingredients." << std::endl;

    const auto dot_txt = std::regex{"\\.nut"};
    auto output_file = std::regex_replace(input_file, dot_txt, ".dat");
    if (output_file == input_file)
      throw std::runtime_error{"output = input: " + output_file};

    auto output = std::ofstream{output_file, std::ios::binary};

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
