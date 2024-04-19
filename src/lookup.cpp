// Copyright 2023-2024 Terry Golubiewski, all rights reserved.

#include "Atwater.h"
#include "To.h"
#include "Parse.h"

#include <system_error>
#include <ranges>
#include <regex>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <chrono>
#include <charconv>
#include <limits>
#include <cstdlib>
#include <cmath>

namespace rng = std::ranges;

std::ios::fmtflags DefaultCoutFlags;

const auto DbPath = std::string{"db/"};

constexpr auto Round(float x) -> float
  { return (std::abs(x) < 10) ? (std::round(10 * x) / 10) : std::round(x); }

auto ToStr(float x) {
  std::array<char, 16> buf;
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), x,
				 std::chars_format::fixed, 2);
  if (ec != std::errc())
    throw std::system_error(std::make_error_code(ec));
  return std::string(buf.data(), ptr);
}

class FdcId {
private:
  gsl::index idx = 0;
public:
  FdcId() : idx{} { }
  explicit FdcId(int id) : idx{id} { }
  operator gsl::index() const { return idx; }
}; // FdcId

std::istream& operator>>(std::istream& is, FdcId& id) {
  is >> std::ws;
  if (is.peek() == '"') {
    std::string str;
    is >> std::quoted(str);
    id = FdcId{To<int>(str)};
  }
  else {
    int idx;
    is >> idx;
    id = FdcId{idx};
  }
  return is;
} // >> FdcId

struct Ingred {
  FdcId id;
  std::string desc;
  float kcal    = 0.0f;
  float protein = 0.0f;
  float fat     = 0.0f;
  float carb    = 0.0f;
  float fiber   = 0.0f;
  float alcohol = 0.0f;
  Atwater atwater;
  friend auto operator<=>(const Ingred&, const Ingred&) = default;
}; // Ingred

std::ostream& operator<<(std::ostream& os, const Ingred& f) {
  std::ostringstream ostr;
  using namespace std;
  ostr << setw(5) << Round(f.kcal)
       << fixed << setprecision(2)
       << ' ' << setw(6) << f.protein
       << ' ' << setw(6) << f.fat
       << ' ' << setw(6) << f.carb
       << ' ' << setw(6) << f.fiber
       << ' ' << setw(6) << f.alcohol
       << ' ' << f.atwater
       << ' ' << f.desc;
  return os << ostr.str();
} // << Ingred

class OutIngred {
  const Ingred& ingred;
public:
  explicit OutIngred(const Ingred& ing_) : ingred{ing_} {
    if (ingred.alcohol != 0.0f && ingred.fiber != 0.0f) {
      throw std::runtime_error{
	  std::to_string(ingred.id) + " invalid alcohol/fiber"};
    }
  }
  friend std::ostream& operator<<(std::ostream& os, const OutIngred& out) {
    std::ostringstream ostr;
    using namespace std;
    const auto& f = out.ingred;
    auto x = (f.alcohol == 0.0f) ? f.fiber : -f.alcohol;
    ostr << setw(5) << Round(f.kcal)
	 << fixed << setprecision(2)
	 << ' ' << setw(6) << f.protein
	 << ' ' << setw(6) << f.fat
	 << ' ' << setw(6) << f.carb
	 << ' ' << setw(6) << x
	 << ' ' << f.desc;
    return os << ostr.str();
  } // << OutIngred
}; // OutIngred

auto ReadFoods()
  -> std::vector<Ingred>
{
  std::vector<Ingred> foods;
  auto input = std::ifstream{"lookup.txt"};
  if (!input)
    throw std::runtime_error("Cannot open lookup.txt");
  Ingred ingred;
  while (input >> ingred.id >> std::ws) {
    if (std::getline(input, ingred.desc))
      foods.push_back(ingred);
  }
  return foods;
} // ReadFoods

void LoadNutrients(std::vector<Ingred>& foods) {
  std::map<FdcId, Ingred*> food_map;
  for (auto& ingred: foods)
    food_map.emplace(ingred.id, &ingred);

  const auto fname = DbPath + "usda_foods.tsv";
  auto db = std::ifstream{fname};
  if (!db)
    throw std::runtime_error("Cannot open " + fname);
  enum class Idx
      { fdc_id, kcal, prot, fat, carb, fiber, alc, atwater, desc, end };
  static const std::array<std::string_view, int(Idx::end)> headings = {
    "fdc_id",
    "kcal",
    "prot",
    "fat",
    "carb",
    "fiber",
    "alc",
    "atwater",
    "desc"
  }; // headings
  std::string line;
  ParseVec<Idx> v;
  if (!std::getline(db, line))
    throw std::runtime_error("Cannot read " + fname);
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  int linenum = 1;
  int found = 0;
  while (std::getline(db, line)) {
    try {
      ++linenum;
      ParseTsv(v, line);
      auto fdc_id = FdcId{To<int>(v[Idx::fdc_id])};
      auto food = food_map.find(fdc_id);
      if (food == food_map.end())
	continue;
      ++found;
      auto ingred = food->second;
      ingred->id      = fdc_id;
      ingred->kcal    = To<float>(v[Idx::kcal]);
      ingred->protein = To<float>(v[Idx::prot]);
      ingred->fat     = To<float>(v[Idx::fat]);
      ingred->carb    = To<float>(v[Idx::carb]);
      ingred->fiber   = To<float>(v[Idx::fiber]);
      ingred->alcohol = To<float>(v[Idx::alc]);
      ingred->atwater = Atwater{v[Idx::atwater]};
      if (ingred->desc.empty())
        ingred->desc    = std::string(v[Idx::desc]);
    }
    catch (std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
} // LoadNutrients

struct Portion {
  FdcId id;
  float g  = 0.0;
  float ml = 0.0;
  std::string desc;
  std::string comment;
  Portion(FdcId id_, float g_, float ml_,
	  std::string desc_, std::string comment_)
    : id{id_}, g{g_}, ml{ml_}
    , desc{std::move(desc_)}, comment{std::move(comment_)}
    { }
  friend auto operator<=>(const Portion&, const Portion&) = default;
}; // Portion

std::ostream& operator<<(std::ostream& os, const Portion& p) {
  return os << '{' << p.id << ' ' << p.g << ' ' << p.ml << ' ' << p.desc << '}';
} // << Portion

auto LoadPortions(const std::vector<Ingred>& foods)
  -> std::vector<Portion>
{
  const auto fname = DbPath + "usda_portions.tsv";
  auto input = std::ifstream(fname);
  if (!input)
    throw std::runtime_error("Cannot open " + fname);
  std::vector<FdcId> fdc_ids;
  fdc_ids.reserve(foods.size());
  rng::transform(foods, std::back_inserter(fdc_ids), &Ingred::id);
  rng::sort(fdc_ids);
  enum class Idx { fdc_id, g, ml, desc, comment, end };
  static const std::array<std::string_view, int(Idx::end)> headings = {
    "fdc_id",
    "g",
    "ml",
    "desc",
    "comment"
  }; // headings
  std::vector<Portion> rval;
  std::string line;
  ParseVec<Idx> v;
  if (!std::getline(input, line))
    throw std::runtime_error("Cannot read " + fname);
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  int linenum = 1;
  while (std::getline(input, line)) {
    try {
      ++linenum;
      ParseTsv(v, line);
      auto fdc_id = FdcId{To<int>(v[Idx::fdc_id])};
      if (!rng::binary_search(fdc_ids, fdc_id))
	continue;
      rval.emplace_back(fdc_id, To<float>(v[Idx::g]), To<float>(v[Idx::ml]),
			std::string{v[Idx::desc]},
			std::string{v[Idx::comment]});
    }
    catch (std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  rng::sort(rval);
  return rval;
} // LoadPortions

class MlText {
private:
  static constexpr float Round(float x) {
    return (std::abs(x) >= 100.0f) ? (std::round(x * 10) / 10)
				   : (std::round(x * 100) / 100);
  }
  std::map<float, std::string> dict;
public:
  MlText() {
    constexpr auto FlOz = 29.5735f;
    constexpr auto Cup  = 8 * FlOz;
    constexpr auto Tbsp = Cup / 16;
    constexpr auto Tsp  = Tbsp / 3;
    constexpr auto Pint = 2 * Cup;
    constexpr auto Quart = 4 * Cup;
    constexpr auto Gallon = 4 * Quart;
    dict.emplace(Round(FlOz),   "FLOZ");
    dict.emplace(Round(Cup),    "CUP");
    dict.emplace(Round(Cup/2),  "HCUP");
    dict.emplace(Round(Cup/4),  "QCUP");
    dict.emplace(Round(Cup/3),  "1_3C");
    dict.emplace(Round(Pint),   "PINT");
    dict.emplace(Round(Quart),  "QT");
    dict.emplace(Round(Gallon), "GAL");
    dict.emplace(Round(Tbsp),   "TBSP");
    dict.emplace(Round(2*Tbsp), "2TBSP");
    dict.emplace(Round(3*Tbsp), "3TBSP");
    dict.emplace(Round(Tsp),    "TSP");
    dict.emplace(Round(2*Tsp),  "2TSP");
    for (int i = 2; i < 16; ++i)
      dict.emplace(Round(i * FlOz), std::to_string(i) + "FLOZ");
  }
  std::string operator()(float x) const {
    x = Round(x);
    if (auto iter = dict.find(x); iter != dict.end())
      return iter->second;
    if (std::abs(x) < 0.05f)
      return "0";
    std::ostringstream oss;
    oss << x;
    return oss.str();
  }
}; // MlText

int main() {
  using namespace std::literals;
  DefaultCoutFlags = std::cout.flags();

  auto foods = ReadFoods();

  LoadNutrients(foods);

  const auto portions = LoadPortions(foods);

  const auto fname = "lookout.txt"s;
  auto output = std::ofstream(fname);
  output << "#include \"defs.txt\"\n";
  if (!output)
    throw std::runtime_error("Could not write " + fname);
  const MlText mlStr;
  output << std::fixed << std::setprecision(2);
  auto last_atwater = Atwater{0, 0, 0, 0};
  for (const auto& ingred: foods) {
    if (ingred.atwater != last_atwater) {
      last_atwater = ingred.atwater;
      output << '[' << last_atwater.str() << "]\n";
    }
    output << "   100     0 " << OutIngred(ingred)
           << " // usda " << ingred.id << '\n';
    {
      auto r = rng::equal_range(portions, ingred.id, rng::less{}, &Portion::id);

      using std::setw, std::left, std::right, std::quoted;

      std::ostringstream ostr;
      for (const auto& p: r) {
	ostr << ((p.ml == 0.0f) ? '*' : ' ') << setw(5) << Round(p.g)
	     << ' ' << setw(5) << mlStr(p.ml)
	     << ' ' << setw(5) << 0
	     << ' ' << left << setw(27) << "this" << right;
	if (!p.desc.empty())
	  ostr << ' ' << p.desc;
	ostr << " $this";
	if (!p.comment.empty())
	  ostr << " // " << p.comment;
	ostr << '\n';
      }
      output << ostr.str();
    }
  }
  return EXIT_SUCCESS;
} // main
