
// Copyright 2024 Terry Golubiewski, all rights reserved.

#include "To.h"
#include "Parse.h"

#include <gsl/gsl>

#include <system_error>
#include <ranges>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <cmath>

namespace rng = std::ranges;

const std::string UsdaPath = "../usda/";
const std::string FdcPath = UsdaPath + "fdc/";
const std::string FnddsPath  = UsdaPath + "fndds/";

std::ios::fmtflags DefaultCoutFlags;

constexpr auto Round(float x) -> float
  { return (std::abs(x) < 10) ? (std::round(10 * x) / 10) : std::round(x); }

class FdcId {
public:
  static constexpr auto Min =   100000;
  static constexpr auto Max = 3'000000 - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"FdcId: " + std::to_string(idx)};
  }
public:
  FdcId() : idx{} { }
  explicit FdcId(int id) : idx(id) { }
  operator gsl::index() const { return idx; }
}; // FdcId

class NdbId {
public:
  static constexpr auto Min =   1'000;
  static constexpr auto Max = 100'000 - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"NdbId: " + std::to_string(idx)};
  }
public:
  NdbId() : idx{} { }
  explicit NdbId(int id) : idx(id) { }
  operator gsl::index() const { return idx; }
}; // NdbId

class FnddsId {
public:
  static constexpr auto Min =  10'000000;
  static constexpr auto Max = 100'000000 - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"FnddsId: " + std::to_string(idx)};
  }
public:
  FnddsId() : idx{} { }
  explicit FnddsId(int id) : idx(id) { check(); }
  operator gsl::index() const { return idx; }
}; // FnddsId

auto GetLegacy() {
  const auto fname = FdcPath + "sr_legacy_food.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx { fdc_id, ndb_id, end };

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "fdc_id",
    "NDB_number"
  }; // headings

  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};

  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  std::map<NdbId, FdcId> rval;
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      auto fdc_id  = FdcId{To<int>(v[Idx::fdc_id])};
      auto ndb_id  = NdbId{To<int>(v[Idx::ndb_id])};
      auto result =rval.emplace(ndb_id, fdc_id);
      if (!result.second)
        throw std::runtime_error{"duplicate " + std::to_string(ndb_id)};
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  std::cout << "Read " << rval.size() << " legacy foods\n";
  return rval;
} // GetLegacy

struct Ingred {
  FnddsId fndds_id;
  NdbId ndb_id;
  std::string desc;
  float kcal    = 0.0f;
  float protein = 0.0f;
  float fat     = 0.0f;
  float carb    = 0.0f;
  float fiber   = 0.0f;
  float alcohol = 0.0f;
  Ingred() = default;
  Ingred(FnddsId id_, NdbId code_, std::string desc_)
    : fndds_id{id_}, ndb_id{code_}, desc{desc_} { }
  Ingred(FnddsId id_, NdbId code_, std::string_view desc_)
    : fndds_id{id_}, ndb_id{code_}, desc{desc_} { }
  auto operator<=>(const Ingred& rhs) const = default;
}; // Ingred

void Update(Ingred& ingred, const auto& nutr_code, const auto& value) {
  if (nutr_code.empty() || value.empty())
    return;

  static const std::array<std::string_view, 6> codes = {
    "203", // protein
    "204", // fat
    "205", // carb
    "208", // kcal
    "221", // alcohol
    "291"  // fiber
  }; // codes
  auto iter = rng::lower_bound(codes, nutr_code);
  if (iter == codes.end() || *iter != nutr_code)
    return;
  auto val = To<float>(value);
  switch (std::distance(codes.begin(), iter)) {
    case 0: ingred.protein  = val; break;
    case 1: ingred.fat      = val; break;
    case 2: ingred.carb     = val; break;
    case 3: ingred.kcal     = val; break;
    case 4: ingred.alcohol  = val; break;
    case 5: ingred.fiber    = val; break;
    default:
      throw std::out_of_range{"Update ingredient nutrient code: " + nutr_code};
  }
} // Update ingred

std::ostream& operator<<(std::ostream& os, const Ingred& f) {
  std::ostringstream ostr;
  using namespace std;
  ostr << fixed << setprecision(2)
              << setw(6) << f.fndds_id
       << ' ' << setw(6) << f.ndb_id
       << ' ' << setw(6) << f.kcal
       << ' ' << setw(6) << f.protein
       << ' ' << setw(6) << f.fat
       << ' ' << setw(6) << f.carb
       << ' ' << setw(6) << f.fiber
       << ' ' << setw(6) << f.alcohol
       << ' ' << f.desc;
  return os << ostr.str();
} // << Ingred

auto GetIngredFoods(const std::map<NdbId, FdcId>& legacy) -> std::map<FnddsId, NdbId> {
  const auto fname = FnddsPath + "fnddsingred.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx {
    fdc_id, seq_num, ingred_code, ingred_desc, amount, measure, portion,
    retention, weight, start_date, end_date,
    end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "Food_code",
    "Seq_num",
    "Ingredient_code",
    "Ingredient_description",
    "Amount",
    "Measure",
    "Portion_code",
    "Retention_code",
    "Ingredient_weight",
    "Start_date",
    "End_date"
  }; // headings

  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};

  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  std::map<FnddsId, NdbId> rval;
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      auto fndds_id = FnddsId{To<int>(v[Idx::fdc_id])};
      auto ndb_id   =   NdbId{To<int>(v[Idx::ingred_code])};
      auto iter = rval.find(fndds_id);
      if (iter == rval.end())
        rval.emplace(fndds_id, ndb_id);
      else
        iter->second = NdbId{};
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  std::cout << "Read " << rval.size() << " ingredient foods\n";
  return rval;
} // GetIngredFoods

auto GetFoods(const std::map<FnddsId, NdbId>& ingredFoods)
  -> std::vector<Ingred>
{
  auto outname = std::string("fndds_food.txt");
  auto output = std::ofstream{outname};
  if (!output)
    throw std::runtime_error{"Cannot write " + outname};
  std::string line;
  const auto fname = FnddsPath + "mainfooddesc.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx {
    fdc_id, desc, cat_num, dat_desc, start_date, end_date, end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "Food_code",
    "Main_food_description",
    "WWEIA_Category_number",
    "WWEIA_Category_description",
    "Start_date",
    "End_date"
  }; // headings

  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  std::vector<Ingred> rval;
  std::cout << "Reading " << fname << '\n';
  {
    long long linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      try {
	ParseTsv(v, line);
	auto fndds_id = FnddsId{To<int>(v[Idx::fdc_id])};
	auto iter = ingredFoods.find(fndds_id);
	NdbId ndb_id;
	if (iter == ingredFoods.end())
	  ;
	else if (iter->second == NdbId{})
	  continue;
	else
	  ndb_id = iter->second;
	rval.emplace_back(fndds_id, ndb_id, v[Idx::desc]);
	output << fndds_id << "\t|" << v[Idx::desc] << '\n';
      }
      catch (const std::exception& x) {
	std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
	std::cerr << line << '\n';
      }
    }
  }
  rng::sort(rval);
  std::cout << "Read " << rval.size() << " foods\n";
  return rval;
} // GetFoods

void ProcessNutrients(std::vector<Ingred>& foods) {
  std::cout << "Processing nutrients.\n";

  enum class Idx {
    fndds_id, ndb_id, amount, start_date, end_date, end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "Food_code",
    "Nutrient_code",
    "Nutrient_value",
    "Start_date",
    "End_date"
  }; // headings

  const std::string fname = FnddsPath + "fnddsnutval.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};
  std::string line;
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  FnddsId last_id;
  Ingred* last_ingred = nullptr;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      auto fndds_id = FnddsId{To<int>(v[Idx::fndds_id])};
      Ingred* ingred = nullptr;
      if (fndds_id == last_id) {
	ingred = last_ingred;
      }
      else {
	last_id = fndds_id;
	auto food = rng::lower_bound(foods, fndds_id, {}, &Ingred::fndds_id);
	if (food == foods.end() || food->fndds_id != fndds_id) {
	  last_ingred = nullptr;
	  continue;
	}
	ingred = last_ingred = &*food;
      }
      if (!ingred)
	continue;
      Update(*ingred, v[Idx::ndb_id], v[Idx::amount]);
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  const std::string outname{"fndds_foods.tsv"};
  auto output = std::ofstream{outname};
  if (!output)
    throw std::runtime_error{"Could not write " + outname};
  output << "fdc_id\tcode\tkcal\tprot\tfat\tcarb\tfiber\talc\tdesc\n";
  output << std::fixed << std::setprecision(2);
  for (const auto& ingred: foods) {
    output << ingred.fndds_id
        << '\t' << ingred.ndb_id
	<< '\t' << ingred.kcal
	<< '\t' << ingred.protein
	<< '\t' << ingred.fat
	<< '\t' << ingred.carb
	<< '\t' << ingred.fiber
	<< '\t' << ingred.alcohol
	<< '\t' << ingred.desc
	<< '\n';
  }
  std::cout << "Wrote " << foods.size() << " foods to " << outname << ".\n";
} // ProcessNutrients

class PortionId {
public:
  static constexpr auto Min =  10'000;
  static constexpr auto Max = 100'000 - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"PortionId: " + std::to_string(idx)};
  }
public:
  PortionId() : idx{} { }
  explicit PortionId(int id) : idx(id) { check(); }
  operator gsl::index() const { return idx; }
}; // PortionId

struct PortionDesc {
  PortionId id;
  std::string desc;
  friend
  auto operator<=>(const PortionDesc& lhs, const PortionDesc& rhs) = default;
}; // PortionDesc

auto GetPortionDesc() {
  std::string line;
  const auto fname = FnddsPath + "foodportiondesc.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx {
    id, desc, start_date, end_date, end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "Portion_code",
    "Portion_description",
    "Start_date",
    "End_date"
  }; // headings

  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  auto rval = std::vector<PortionDesc>{};
  std::cout << "Reading " << fname << '\n';
  {
    long long linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      try {
	ParseTsv(v, line);
	rval.emplace_back(PortionId{To<int>(v[Idx::id])},
	                  To<std::string>(v[Idx::desc]));
      }
      catch (const std::exception& x) {
	std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
	std::cerr << line << '\n';
      }
    }
  }
  rng::sort(rval);
  std::cout << "Read " << rval.size() << " portion descriptions\n";
  return rval;
} // GetPortionDesc

struct Portion {
  gsl::index ingred;
  float g;
  gsl::index desc;
  friend
  auto operator<=>(const Portion& lhs, const  Portion& rhs) = default;
}; // Portion

auto GetPortions(const std::vector<Ingred>& foods,
                 const std::vector<PortionDesc>& desc)
{
  std::string line;
  const auto fname = FnddsPath + "foodweights.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx {
    food, seqn, portion, weight, start_date, end_date, end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "Food_code",
    "Seq_num",
    "Portion_code",
    "Portion_weight",
    "Start_date",
    "End_date"
  }; // headings

  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  auto rval = std::vector<Portion>{};
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      const auto food_id    = FnddsId{To<int>(v[Idx::food])};
      const auto portion_id = PortionId{To<int>(v[Idx::portion])};
      auto food_iter =
	  rng::lower_bound(foods, food_id, rng::less{}, &Ingred::fndds_id);
      if (food_iter == foods.end() || food_iter->fndds_id != food_id)
        continue;
      auto food = gsl::index{std::distance(foods.begin(), food_iter)};
      auto portion_iter =
	  rng::lower_bound(desc, portion_id, rng::less{}, &PortionDesc::id);
      if (portion_iter == desc.end() || portion_iter->id != portion_id) {
	throw std::runtime_error{
	    "Portion description not found: " + std::string{v[Idx::portion]}};
      }
      auto portion = gsl::index{std::distance(desc.begin(), portion_iter)};
      rval.emplace_back(food, To<float>(v[Idx::weight]), portion);
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
      std::cerr << line << '\n';
    }
  }
  rng::sort(rval);
  std::cout << "Read " << linenum << " portions, discarded "
            << (linenum - rval.size()) << '\n';
  return rval;
} // GetPortions

int main() {
  DefaultCoutFlags = std::cout.flags();
  std::cout << "Starting..." << std::endl;
  auto foods = GetFoods(GetIngredFoods(GetLegacy()));
  ProcessNutrients(foods);
  const auto portionDesc = GetPortionDesc();
  auto portions = GetPortions(foods, portionDesc);
  auto fname = std::string{"fndds_portions.tsv"};
  auto output = std::ofstream{fname, std::ios::binary};
  if (!output)
    throw std::runtime_error{"Cannot write " + fname};
  for (const auto& p: portions) {
    const auto& food = foods[p.ingred];
    const auto& portion = portionDesc[p.desc];
    output << food.fndds_id << '\t' << p.g << '\t' << portion.desc << '\n';
  }
  return EXIT_SUCCESS;
} // main
