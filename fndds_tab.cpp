
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
  static constexpr auto Min =   150000;
  static constexpr auto Max = 3'000000 - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"FdcId: " + To<std::string>(idx)};
  }
public:
  FdcId() : idx{} { }
  explicit FdcId(int id) : idx(id) { check(); }
  operator gsl::index() const { return idx; }
}; // FdcId

class NdbId {
public:
  static constexpr auto Min =   1'000;
  static constexpr auto Max = FdcId::Min - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"NdbId: " + To<std::string>(idx)};
  }
public:
  NdbId() : idx{} { }
  explicit NdbId(int id) : idx(id) { check(); }
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
      throw std::range_error{"FnddsId: " + To<std::string>(idx)};
  }
public:
  FnddsId() : idx{} { }
  explicit FnddsId(int id) : idx(id) { check(); }
  operator gsl::index() const { return idx; }
}; // FnddsId

struct Food {
  FnddsId fndds_id;
  NdbId ndb_id;
  std::string desc;
  float kcal    = 0.0f;
  float protein = 0.0f;
  float fat     = 0.0f;
  float carb    = 0.0f;
  float fiber   = 0.0f;
  float alcohol = 0.0f;
  Food() = default;
  Food(FnddsId id_, NdbId code_, std::string desc_)
    : fndds_id{id_}, ndb_id{code_}, desc{desc_} { }
  Food(FnddsId id_, NdbId code_, std::string_view desc_)
    : fndds_id{id_}, ndb_id{code_}, desc{desc_} { }
  auto operator<=>(const Food& rhs) const = default;
}; // Food

void Update(Food& food, const auto& nutr_code, const auto& value) {
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
    case 0: food.protein  = val; break;
    case 1: food.fat      = val; break;
    case 2: food.carb     = val; break;
    case 3: food.kcal     = val; break;
    case 4: food.alcohol  = val; break;
    case 5: food.fiber    = val; break;
    default:
      throw std::out_of_range{"Update ingredient nutrient code: " + nutr_code};
  }
} // Update food

std::ostream& operator<<(std::ostream& os, const Food& f) {
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
} // << Food

class AnyId: public std::variant<NdbId, FdcId, FnddsId> {
private:
  using base_type = std::variant<NdbId, FdcId, FnddsId>;
  constexpr static auto Select(int id) -> base_type {
    if (id >= NdbId::Min && id <= NdbId::Max)
      return NdbId{id};
    if (id >= FdcId::Min && id <= FdcId::Max)
      return FdcId{id};
    return FnddsId{id};
  } // Select
  base_type& base() { return *static_cast<base_type*>(this); }
  const base_type& base() const { return *static_cast<const base_type*>(this); }
public:
  constexpr AnyId() noexcept : base_type{} { }
  constexpr explicit AnyId(int id) : base_type{Select(id)} { }
  template<class T>
  constexpr explicit AnyId(T&& t) noexcept : base_type{std::forward<T>(t)} { }
  template<class T>
  AnyId& operator=(T x) { base().operator=(x); return *this; }
  int value() const {
    switch (index()) {
      case 0: return std::get<NdbId>(*this);
      case 1: return std::get<FdcId>(*this);
      case 2: return std::get<FnddsId>(*this);
      default: return 0;
    }
  }
}; // AnyId

std::ostream& operator<<(std::ostream& os, const AnyId& x) {
  switch (x.index()) {
    case 0: os << "NdbId{" << std::get<NdbId>(x) << '}'; break;
    case 1: os << "FdcId{" << std::get<FdcId>(x) << '}'; break;
    case 2: os << "FnddsId{" << std::get<FnddsId>(x) << '}'; break;
    default: os << "AnyId{?}" ; break;
  }
  return os;
} // << AnyId

auto GetIngredFoods()
  -> std::map<FnddsId, AnyId>
{
  const auto fname = FnddsPath + "fnddsingred.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx {
    fdc_id, seq_num, ingred_code, ingred_desc, amount, measure, portion,
    retention, weight, start_date, end_date,
    end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> Headings = {
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
  }; // Headings

  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};

  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
  std::map<FnddsId, AnyId> rval;
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      const auto fndds_id = FnddsId{To<int>(v[Idx::fdc_id])};
      const auto iter = rval.find(fndds_id);
      if (iter == rval.end()) {
        auto id = To<int>(v[Idx::ingred_code]);
	if (id < FnddsId::Min) {
	  if (id >= 1000000) {
	    throw std::range_error{
		"Invalid ingredient code: " + To<std::string>(id)};
	  }
	  if (id >=  999000)
	    continue;
	  if (id >=  900000)
	    id -= 900000;
	}
        rval.emplace(fndds_id, AnyId{id});
      }
      else {
        iter->second = AnyId{};
      }
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  for (auto& r: rval) {
    auto& id = r.second;
    while (id.index() == 2) {
      auto iter = rval.find(std::get<FnddsId>(id));
      if (iter == rval.end())
        break;
      id = iter->second;
    }
  }
#if 0
  {
    auto is_zero = [](const auto& x) { return (x.second == AnyId{}); };
    (void) std::erase_if(rval, is_zero);
  }
#endif
  std::cout << "Read " << rval.size() << " ingredient foods\n";
#if 0
  {
    const auto outname = std::string{"fndds_ingred.tsv"};
    auto output = std::ofstream{outname, std::ios::binary};
    if (!output)
      throw std::runtime_error{"Cannot write to " + outname};
    for (const auto& [fndds_id, any_id]: rval)
      output << fndds_id << '\t' << any_id << '\n';
  }
#endif
  return rval;
} // GetIngredFoods

auto GetFoods(const std::map<FnddsId, AnyId>& ingredFoods)
  -> std::vector<Food>
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

  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "Food_code",
    "Main_food_description",
    "WWEIA_Category_number",
    "WWEIA_Category_description",
    "Start_date",
    "End_date"
  }; // Headings

  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
  std::vector<Food> rval;
  std::cout << "Reading " << fname << '\n';
  {
    long long linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      try {
	ParseTsv(v, line);
	const auto fndds_id = FnddsId{To<int>(v[Idx::fdc_id])};
	const auto iter = ingredFoods.find(fndds_id);
	NdbId ndb_id;
	if (iter == ingredFoods.end())
	  ;
	else if (iter->second == AnyId{})
	  continue;
	else
	  ndb_id = std::get<NdbId>(iter->second);
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

auto GetLegacy() {
  const auto fname = FdcPath + "sr_legacy_food.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx { fdc_id, ndb_id, end };
  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "fdc_id",
    "NDB_number"
  }; // Headings

  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};

  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
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

auto GetSurvey() -> std::map<FnddsId, FdcId> {
  const auto fname = FdcPath + "survey_fndds_food.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};

  enum class Idx { fdc_id, fndds_id, wweia_id, start_date, end_date, end };
  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "fdc_id",
    "food_code",
    "wweia_category_code",
    "start_date",
    "end_date"
  }; // Headings

  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};

  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
  std::map<FnddsId, FdcId> rval;
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      auto fdc_id   =   FdcId{To<int>(v[Idx::fdc_id])};
      auto fndds_id = FnddsId{To<int>(v[Idx::fndds_id])};
      auto result =rval.emplace(fndds_id, fdc_id);
      if (!result.second)
        throw std::runtime_error{"duplicate " + std::to_string(fndds_id)};
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  std::cout << "Read " << rval.size() << " survey foods\n";
  return rval;
} // GetSurvey

FdcId ToFdcId(NdbId id) {
  static const auto legacy = GetLegacy();
  const auto iter = legacy.find(id);
  if (iter == legacy.end())
    return FdcId{};
  return iter->second;
}

FdcId ToFdcId(FnddsId id) {
  static const auto survey = GetSurvey();
  const auto iter = survey.find(id);
  if (iter == survey.end())
    return FdcId{};
  return iter->second;
}

FdcId ToFdcId(const AnyId& id) {
  switch (id.index()) {
  case 0: return ToFdcId(std::get<NdbId>(id));
  case 1: return std::get<FdcId>(id);
  case 2: return ToFdcId(std::get<FnddsId>(id));
  default: return FdcId{};
  }
} // ToFdcId

void ProcessNutrients(std::vector<Food>& foods) {
  std::cout << "Processing nutrients.\n";

  enum class Idx {
    fndds_id, nutr_id, amount, start_date, end_date, end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "Food_code",
    "Nutrient_code",
    "Nutrient_value",
    "Start_date",
    "End_date"
  }; // Headings

  const std::string fname = FnddsPath + "fnddsnutval.tsv";
  auto input = std::ifstream{fname};
  if (!input)
    throw std::runtime_error{"Cannot open " + fname};
  std::string line;
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
  std::cout << "Reading " << fname << '\n';
  long long linenum = 1;
  FnddsId last_id;
  Food* last_food = nullptr;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      auto fndds_id = FnddsId{To<int>(v[Idx::fndds_id])};
      Food* food_ptr = nullptr;
      if (fndds_id == last_id) {
	food_ptr = last_food;
      }
      else {
	last_id = fndds_id;
	auto food = rng::lower_bound(foods, fndds_id, {}, &Food::fndds_id);
	if (food == foods.end() || food->fndds_id != fndds_id) {
	  last_food = nullptr;
	  continue;
	}
	food_ptr = last_food = &*food;
      }
      if (!food_ptr)
	continue;
      Update(*food_ptr, v[Idx::nutr_id], v[Idx::amount]);
    }
    catch (const std::exception& x) {
      std::cerr << fname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  {
    const std::string outname{"fndds_foods.tsv"};
    auto output = std::ofstream{outname};
    if (!output)
      throw std::runtime_error{"Could not write " + outname};
    output << "fdc_id\tkcal\tprot\tfat\tcarb\tfiber\talc\tdesc\n";
    output << std::fixed << std::setprecision(2);
    for (const auto& food: foods) {
      auto any_id = AnyId{ToFdcId(food.ndb_id)};
      if (std::get<FdcId>(any_id) == FdcId{})
        any_id = food.ndb_id;
      output << any_id.value()
	  << '\t' << food.kcal
	  << '\t' << food.protein
	  << '\t' << food.fat
	  << '\t' << food.carb
	  << '\t' << food.fiber
	  << '\t' << food.alcohol
	  << '\t' << food.desc
	  << '\n';
    }
    std::cout << "Wrote " << foods.size() << " foods to " << outname << ".\n";
  }
} // ProcessNutrients

class PortionId {
public:
  static constexpr auto Min =  10'000;
  static constexpr auto Max = 100'000 - 1;
private:
  gsl::index idx = 0;
  void check() {
    if (idx < Min || idx > Max)
      throw std::range_error{"PortionId: " + To<std::string>(idx)};
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

  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "Portion_code",
    "Portion_description",
    "Start_date",
    "End_date"
  }; // Headings

  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
  auto rval = std::vector<PortionDesc>{};
  std::cout << "Reading " << fname << '\n';
  {
    long long linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      try {
	ParseTsv(v, line);
	const auto id = To<int>(v[Idx::id]);
	if (id == 0)
	  continue;
	rval.emplace_back(PortionId{id}, To<std::string>(v[Idx::desc]));
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
  gsl::index food;
  float g;
  gsl::index desc;
  friend
  auto operator<=>(const Portion& lhs, const  Portion& rhs) = default;
}; // Portion

auto GetPortions(const std::vector<Food>& foods,
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

  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "Food_code",
    "Seq_num",
    "Portion_code",
    "Portion_weight",
    "Start_date",
    "End_date"
  }; // Headings

  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + fname};
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
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
	  rng::lower_bound(foods, food_id, rng::less{}, &Food::fndds_id);
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
  auto foods = GetFoods(GetIngredFoods());
  ProcessNutrients(foods);
  const auto portionDesc = GetPortionDesc();
  auto portions = GetPortions(foods, portionDesc);
  auto fname = std::string{"fndds_portions.tsv"};
  auto output = std::ofstream{fname, std::ios::binary};
  if (!output)
    throw std::runtime_error{"Cannot write " + fname};
  for (const auto& p: portions) {
    const auto& food = foods[p.food];
    const auto& portion = portionDesc[p.desc];
    output << food.fndds_id << '\t' << p.g << '\t' << portion.desc << '\n';
  }
  return EXIT_SUCCESS;
} // main
