// Copyright 2023-2024 Terry Golubiewski, all rights reserved.

#include "Atwater.h"
#include "To.h"
#include "Parse.h"

#include <gsl/gsl>

#include <system_error>
#include <regex>
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
#include <chrono>
#include <charconv>
#include <limits>
#include <cstdlib>
#include <cmath>

namespace rng = std::ranges;

const std::string UsdaPath = "../usda/";
const std::string FdcPath  = UsdaPath + "fdc/";
const std::string SrPath   = UsdaPath + "sr/";

std::ios::fmtflags DefaultCoutFlags;

constexpr auto Round(float x) -> float
  { return (std::abs(x) < 10) ? (std::round(10 * x) / 10) : std::round(x); }

class StringDb {
public:
  using Index = gsl::index;
private:
  std::vector<std::string> strings;
  std::map<std::string, Index> known;
public:
  int size() const { return strings.size(); }
  const std::string& str(Index idx) const { return strings.at(idx); }
  Index get(const std::string& str) {
    auto iter = known.find(str);
    if (iter != known.end())
      return iter->second;
    auto idx = size();
    known.emplace(str, idx);
    strings.emplace_back(str);
    return idx;
  } // get
  StringDb() { (void) get(""); }
}; // StringDb

std::ostream& operator<<(std::ostream& os, const StringDb& db) {
  for (int i = 0; i != db.size(); ++i)
    os << db.str(i) << '\n';
  return os;
}

class FdcId {
private:
  gsl::index idx = 0;
public:
  FdcId() : idx{} { }
  explicit FdcId(int id) : idx(id) { }
  operator gsl::index() const { return idx; }
}; // FdcId

enum class FieldIdx {
  kJ, energy, atwater_general, atwater_specific, protein, fat,
  carb_diff, carb_sum, fiber, alcohol, end
}; // FieldIdx

const std::vector<std::string> FieldIds = {
  "1062", // kJ
  "1008", // kcal
  "2047", // atwater general
  "2048", // atwater specific
  "1003", // protein
  "1004", // fat
  "1005", // carb by diff
  "1050", // carb by sum
  "1079", // fiber
  "1018"  // alcohol
}; // FieldIds

using FieldValues = std::array<float, std::size_t(FieldIdx::end)>;

struct AtwaterDb: public StringDb { };
using AtwaterId = AtwaterDb::Index;

struct Ingred {
  FdcId id;
  std::string desc;
  FieldValues values;
  AtwaterId atwater = 0;
  float value(FieldIdx field) const { return values[gsl::index(field)]; }
  float& value(FieldIdx field) { return values[gsl::index(field)]; }
  Ingred() { values.fill(0.0f); }
  explicit Ingred(FdcId id_) : id(id_) { values.fill(0.0f); }
  Ingred(FdcId id_, std::string desc_)
    : id(id_), desc(std::move(desc_)) { values.fill(0.0f); }
  float energy() const {
    auto kcal = value(FieldIdx::atwater_specific);
    if (kcal != 0.0)
      return kcal;
    kcal = value(FieldIdx::atwater_general);
    if (kcal != 0.0)
      return kcal;
    kcal = value(FieldIdx::energy);
    if (kcal != 0.0)
      return kcal;
    constexpr auto KcalPerKj = 0.239;
    return KcalPerKj * value(FieldIdx::kJ);
  } // energy
  float carb()    const {
    auto g = value(FieldIdx::carb_diff);
    if (g != 0.0)
      return g;
    return value(FieldIdx::carb_sum);
  }
  float protein() const { return value(FieldIdx::protein); }
  float fat()     const { return value(FieldIdx::fat);     }
  float fiber()   const { return value(FieldIdx::fiber);   }
  float alcohol() const { return value(FieldIdx::alcohol); }
  friend auto operator<=>(const Ingred& lhs, const Ingred& rhs)
    { return (lhs.id <=> rhs.id); }
}; // Ingred

std::ostream& operator<<(std::ostream& os, const Ingred& f) {
  std::ostringstream ostr;
  using namespace std;
  ostr << setw(5) << Round(f.energy())
       << fixed << setprecision(2)
       << ' ' << setw(6) << f.protein()
       << ' ' << setw(6) << f.fat()
       << ' ' << setw(6) << f.carb()
       << ' ' << setw(6) << f.fiber()
       << ' ' << setw(6) << f.alcohol()
       << ' ' << f.desc;
  return os << ostr.str();
} // << Ingred

auto GetFoods() -> std::vector<Ingred> {
  auto outname = std::string("food.txt");
  auto output = std::ofstream{outname, std::ios::binary};
  if (!output)
    throw std::runtime_error("Cannot write " + outname);
  std::string line;
  enum class Idx { fdc_id, data_type, desc, category, pub_date, end };
  static const std::array<std::string_view, 5> headings = {
    "fdc_id",
    "data_type",
    "description",
    "food_category_id",
    "publication_date"
  }; // headings
  const auto fname = FdcPath + "food.tsv";
  auto input = std::ifstream(fname);
  if (!input)
    throw std::runtime_error("Cannot open " + fname);
  if (!std::getline(input, line))
    throw std::runtime_error("Cannot read " + fname);
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
	if (v.size() != std::size_t(Idx::end))
	  throw std::range_error("Invalid # columns read: "
				  + std::to_string(v.size()));
	const auto& data_type = v[Idx::data_type];
	if (data_type != "foundation_food" && data_type != "sr_legacy_food")
	  continue;
	rval.emplace_back(FdcId{To<int>(v[Idx::fdc_id])}, v[Idx::desc]);
	output << v[Idx::fdc_id] << "\t|" << v[Idx::desc] << '\n';
      }
      catch (const std::exception& x) {
	std::cerr << '\r' << fname << '(' << linenum << ") " << x.what() << '\n';
	std::cerr << line << '\n';
      }
    }
  }
  std::sort(rval.begin(), rval.end());
  std::cout << "Read " << rval.size() << " foods\n";
  return rval;
} // GetFoods

auto AtwaterString(const std::string& prot, const std::string& fat,
                   const std::string& carb)
  -> std::string
{
  if (prot.empty() && fat.empty() && carb.empty())
    return "";
  return prot + "," + fat + "," + carb;
} // AtwaterString

void ReadAtwaterFoods(std::vector<Ingred>& foods, AtwaterDb& atwaterDb) {
  std::map<std::string, AtwaterId> atwaterCodes;
  std::string line;
  {
    enum class Idx { id, protein, fat, carb, end };
    auto fname = FdcPath + "food_calorie_conversion_factor.tsv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    ParseTsv(v, line);
    if (v.size() != std::size_t(Idx::end)
        || v[Idx::id]      != "food_nutrient_conversion_factor_id"
	|| v[Idx::protein] != "protein_value"
	|| v[Idx::fat]     != "fat_value"
	|| v[Idx::carb]    != "carbohydrate_value")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      ParseTsv(v, line);
      const auto& id = v[Idx::id];
      auto atwater =
	  AtwaterString(v[Idx::protein], v[Idx::fat], v[Idx::carb]);
      atwaterCodes.emplace(id, atwaterDb.get(atwater));
    }
    std::cout << "Read " << atwaterCodes.size() << " Atwater codes ("
	<< atwaterDb.size() << " unique).\n";
  }
  {
    enum Idx { id, fdc_id, end };
    auto fname = FdcPath + "food_nutrient_conversion_factor.tsv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    ParseTsv(v, line);
    if (v.size() != std::size_t(Idx::end)
	|| v[Idx::id]     != "id"
	|| v[Idx::fdc_id] != "fdc_id")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      ParseTsv(v, line);
      auto iter = atwaterCodes.find(v[Idx::id]);
      if (iter == atwaterCodes.end())
        continue;
      auto id = FdcId{To<int>(v[Idx::fdc_id])};
      auto food = rng::lower_bound(foods, id, {}, &Ingred::id);
      if (food == foods.end() || food->id != id)
        continue;
      food->atwater = iter->second;
    }
  }
} // ReadAtwaterFoods

void UpdateAtwaterFromLegacy(std::vector<Ingred>& foods, AtwaterDb& atwaterDb) {
  std::cout << "Reading legacy Atwater codes.\n";
  std::string line;
  std::map<std::string, Ingred*> legacy;
  {
    enum class Idx { fdc_id, NDB_number, end };
    auto fname = FdcPath + "sr_legacy_food.tsv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    ParseTsv(v, line);
    if (v.size() != std::size_t(Idx::end)
        || v[Idx::fdc_id]     != "fdc_id"
	|| v[Idx::NDB_number] != "NDB_number")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      ParseTsv(v, line);
      auto fdc_id = FdcId{To<int>(v[Idx::fdc_id])};
      auto food = rng::lower_bound(foods, fdc_id, {}, &Ingred::id);
      if (food == foods.end() || food->id != fdc_id)
        continue;
      legacy.emplace(v[Idx::NDB_number], &*food);
    }
  }
  std::cout << "Found " << legacy.size() << " legacy foods.\n";
  {
    enum class Idx {
      NDB_No, FdGrp_Cd, Long_Desc, Shrt_Desc, ComName, ManufacName, Survey,
      Ref_desc, Refuse, SciName, N_Factor, Pro_Factor, Fat_Factor, CHO_Factor,
      end
    };
    auto fname = SrPath + "FOOD_DES.tsv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    int updateCount = 0;
    int linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      try {
	ParseTsv<Idx>(v, line);
	if (v.size() != std::size_t(Idx::end)) {
	  throw std::runtime_error(
	      "invalid # columns: " + std::to_string(v.size()));
	}
	auto iter = legacy.find(v[Idx::NDB_No]);
	if (iter == legacy.end())
	  continue;
	auto& ingred = *iter->second;
	if (ingred.atwater != 0)
	  continue;
	auto str = AtwaterString(v[Idx::Pro_Factor],
	                         v[Idx::Fat_Factor],
				 v[Idx::CHO_Factor]);
	ingred.atwater = atwaterDb.get(str);
	++updateCount;
      }
      catch (const std::exception& x) {
        std::cout << fname << '(' << linenum << ") " << x.what() << '\n';
      }
    }
    std::cout << "Updated " << updateCount << " Atwater codes.\n";
  }
} // UpdateAtwaterFromLegacy

void ProcessNutrients(std::vector<Ingred>& foods) {
  std::cout << "Processing nutrients.\n";

  AtwaterDb atwaterDb;
  ReadAtwaterFoods(foods, atwaterDb);

  UpdateAtwaterFromLegacy(foods, atwaterDb);

  enum class Idx {
    id, fdc_id, nutrient_id, amount, data_points, derivation_id,
    min, max, median, log, footnote, min_year_acquired, end
  }; // Idx

  static const std::array<std::string_view, int(Idx::end)> headings = {
    "id",
    "fdc_id",
    "nutrient_id",
    "amount",
    "data_points",
    "derivation_id",
    "min",
    "max",
    "median",
    "loq",
    "footnote",
    "min_year_acquired"
  }; // headings

  std::string fname = FdcPath + "food_nutrient.tsv";
  auto db = std::ifstream{fname};
  if (!db)
    throw std::runtime_error("Cannot open " + fname);
  std::string line;
  ParseVec<Idx> v;
  {
    if (!std::getline(db, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseTsv(v, line);
    CheckHeadings(v, headings);
  }
  std::cout << "Reading " << fname << '\n';
  {
    long long linenum = 0;
    FdcId last_id;
    Ingred* last_ingred = nullptr;
    while (std::getline(db, line)) {
      ++linenum;
      ParseTsv(v, line);
      auto id = FdcId{To<int>(v[Idx::fdc_id])};
      Ingred* ingred = nullptr;
      if (id == last_id) {
	ingred = last_ingred;
      }
      else {
	last_id = id;
	auto food = rng::lower_bound(foods, id, {}, &Ingred::id);
	if (food == foods.end() || food->id != id) {
	  last_ingred = nullptr;
	  continue;
	}
	ingred = last_ingred = &*food;
      }
      if (!ingred)
	continue;
      auto iter = rng::find(FieldIds, v[Idx::nutrient_id]);
      if (iter == FieldIds.end())
	continue;
      auto i = FieldIdx(std::distance(FieldIds.begin(), iter));
      ingred->value(i) = To<float>(v[Idx::amount]);
    }
  }
  const std::string outname{"usda_foods.tsv"};
  auto output = std::ofstream{outname, std::ios::binary};
  if (!output)
    throw std::runtime_error("Could not write " + outname);
  output << "fdc_id\tkcal\tprot\tfat\tcarb\tfiber\talc\tatwater\tdesc\n";
  output << std::fixed << std::setprecision(2);
  for (const auto& ingred: foods) {
    output << ingred.id
	<< '\t' << ingred.energy()
	<< '\t' << ingred.protein()
	<< '\t' << ingred.fat()
	<< '\t' << ingred.carb()
	<< '\t' << ingred.fiber()
	<< '\t' << ingred.alcohol()
	<< '\t' << atwaterDb.str(ingred.atwater)
	<< '\t' << ingred.desc
	<< '\n';
  }
  std::cout << "Wrote " << foods.size() << " foods to " << outname << ".\n";
} // ProcessNutrients

constexpr float Cup = 236.6;

const std::map<std::string, float> FactorMap = {
  { "cup",         Cup     },
  { "tablespoon",  Cup/16  },
  { "tbsp",        Cup/16  },
  { "Tablespoons", Cup/16  },
  { "teaspoon",    Cup/48  },
  { "tsp",         Cup/48  },
  { "liter",       1000.0  },
  { "milliliter",  1.0     },
  { "ml",          1.0     },
  { "cubic inch",  16.39   },
  { "cubic centimeter", 1.0 },
  { "cc",              1.0 },
  { "gallon",     16 * Cup },
  { "pint",        2 * Cup },
  { "fl oz",       Cup / 8 },
  { "floz",        Cup / 8 },
  { "quart",       4 * Cup }
}; // FactorMap

float ConversionFactor(const std::string& unit) {
  if (unit.empty())
    return 0.0f;
  if (auto iter = FactorMap.find(unit); iter != FactorMap.end())
    return iter->second;
  return 0.0f;
} // ConversionFactor

void ProcessPortions(const std::vector<Ingred>& foods) {
  std::cout << "Processing portions.\n";
  struct Unit {
    const std::string name;
    float ml_factor = 0.0;
    explicit Unit(const std::string& n)
      : name{n}, ml_factor{ConversionFactor(n)} { }
    Unit() { }
  }; // Unit
  static const Unit NullUnit;
  std::map<std::string, Unit> units;
  units.emplace("9999", NullUnit);
  std::string line;
  {
    enum class Idx { id, name, end };
    const auto fname = FdcPath + "measure_unit.tsv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line)) // discard headings
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    ParseTsv(v, line);
    if (v.size() != std::size_t(Idx::end)
      || v[Idx::id]   != "id"
      || v[Idx::name] != "name")
    throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      ParseTsv(v, line);
      if (v[Idx::id] != "9999")
	units.emplace(v[Idx::id], Unit(v[Idx::name]));
    }
    std::cout << "Loaded " << units.size() << " units of measure.\n";
  }
  {
    std::string outname = "usda_portions.tsv";
    auto output = std::ofstream{outname, std::ios::binary};
    if (!output)
      throw std::runtime_error("Cannot write to " + outname);
    enum class Idx { id, fdc_id, seq_num, amount, unit, desc,
	modifier, grams, data_points, footnote, min_year_acquired, end };
    const auto fname = FdcPath + "food_portion.tsv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line)) // discard headings
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    ParseTsv(v, line);
    if (v.size() != std::size_t(Idx::end)
	|| v[Idx::fdc_id]   != "fdc_id"
	|| v[Idx::grams]    != "gram_weight"
	|| v[Idx::amount]   != "amount"
	|| v[Idx::unit]     != "measure_unit_id"
	|| v[Idx::desc]     != "portion_description"
	|| v[Idx::modifier] != "modifier")
      throw std::runtime_error(fname + ": invalid headings");
    FdcId last_fdc_id;
    bool known_fdc_id = false;
    output << "fdc_id\tg\tml\tdesc\tcomment\n";
    output << std::fixed << std::setprecision(2);
    int count = 0;
    while (std::getline(input, line)) {
      ParseTsv(v, line);
      auto fdc_id = FdcId{To<int>(v[Idx::fdc_id])};
      if (fdc_id != last_fdc_id) {
        last_fdc_id = fdc_id;
	auto food = rng::lower_bound(foods, fdc_id, {}, &Ingred::id);
	known_fdc_id = (food != foods.end() && food->id == fdc_id);
      }
      if (!known_fdc_id)
        continue;
      float ml = 0.0f;
      auto g  = To<float>(v[Idx::grams]);
      std::ostringstream desc;
      const auto& amount = v[Idx::amount];
      float val = amount.empty() ? 0.0 : To<float>(amount);
      auto iter = units.find(v[Idx::unit]);
      const auto& unit = (iter != units.end()) ? iter->second : NullUnit;
      if (!unit.name.empty()) {
        if (unit.ml_factor) {
	  ml = val * unit.ml_factor;
	}
	else {
	  if (val != 0.0f && val != 1.0f)
	    desc << val << "x ";
	  desc << unit.name;
	}
	val = 0.0f;
      }
      auto modifier{v[Idx::modifier]};
      if (val != 0.0f && !modifier.empty()) {
        auto pos = modifier.find(',');
	ml = val * ConversionFactor(modifier.substr(0, pos));
	if (ml != 0.0f) {
	  val = 0.0f;
	  constexpr auto npos = std::string::npos;
	  modifier = (pos != npos && pos+1 < v[Idx::modifier].size())
	           ? v[Idx::modifier].substr(pos+1)
	           : std::string{};
	  if (!modifier.empty()) {
	    pos = modifier.find_first_not_of(" ");
	    modifier = (pos != npos) ? modifier.substr(pos) : std::string{};
	  }
	}
	else {
	  auto m = std::string_view(modifier);
	  for (const auto& [unit, factor]: FactorMap) {
	    if (m.starts_with(unit)) {
	      ml = val * factor;
	      val = 0.0f;
	      m.remove_prefix(unit.size());
	      if (!m.empty() && m[0] == ',')
	        m.remove_prefix(1);
	      auto pos = m.find_first_not_of(" ");
	      if (pos != std::string_view::npos)
	        m.remove_prefix(pos);
	      modifier = std::string(m);
	      break;
	    }
	  }
	}
      }
      if (val != 0.0f && val != 1.0f)
        desc << val << 'x';
      if (!v[Idx::desc].empty()) {
        auto d = std::string_view(v[Idx::desc]);
        if (amount.empty() && ml == 0.0f) {
	  float value = 1.0f;
	  if (std::isdigit(d[0])) {
	    std::size_t pos = 0;
	    value = std::stof(v[Idx::desc], &pos);
	    pos = d.find_first_not_of(" ", pos);
	    if (pos != std::string_view::npos)
	      d.remove_prefix(pos);
	  }
	  for (const auto& [unit, factor]: FactorMap) {
	    if (d.starts_with(unit)) {
	      ml = value * factor;
	      d.remove_prefix(unit.size());
	      if (!d.empty() && d[0] == ',')
	        d.remove_prefix(1);
	      auto pos = d.find_first_not_of(" ");
	      if (pos != std::string_view::npos)
	        d.remove_prefix(pos);
	      break;
	    }
	  }
	  if (ml == 0.0f)
	    d = std::string_view(v[Idx::desc]);
	}
        if (!desc.view().empty())
	  desc << ' ';
	desc << d;
      }
      std::string comment;
      if (!modifier.empty()) {
        std::smatch m;
        static const std::regex e{"\\(.*\\)"};
	if (std::regex_search(modifier, m, e)) {
	  static const std::regex e1{" *$"};
	  auto pfx = std::regex_replace(m.prefix().str(), e1, "");
	  static const std::regex e2{"^ *"};
	  auto sfx = std::regex_replace(m.suffix().str(), e2, "");
	  comment = modifier.substr(m.position(0)+1, m.length(0)-2);
	  modifier = pfx;
	  if (!sfx.empty())
	    modifier += " " + sfx;
	}
      }
      if (!modifier.empty()) {
        if (!desc.view().empty())
	  desc << ' ';
	desc << modifier;
      }
      constexpr auto GramsPerOz = 28.34952f;
      constexpr auto GramsPerLb = 16 * GramsPerOz;
      if (ml == 0.0f) {
        if (desc.view() == "oz" && std::abs(g-GramsPerOz)/GramsPerOz < 0.02)
	  continue;
	if (desc.view() == "lb" && std::abs(g-GramsPerLb)/GramsPerLb < 0.02)
	  continue;
	std::smatch m;
	static const std::regex e{"([0-9.]+)x (oz|lb)"};
	auto str = desc.str();
	if (std::regex_match(str, m, e)) {
	  auto valstr = m.str(1);
	  float v = To<float>(m.str(1));
	  if (v != 0.0f) {
	    auto expect = v * ((m[2] == "oz") ? GramsPerOz : GramsPerLb);
	    if (std::abs(g - expect) / expect < 0.02)
	      continue;
	  }
	}
      }
      using std::setw;
      output << setw(6) << fdc_id
	     << '\t' << setw(6) << g
             << '\t' << setw(6) << ml
	     << '\t' << desc.view()
	     << '\t' << comment
	     << '\n';
      ++count;
    }
    std::cout << "Wrote " << count << " portions to " << outname << ".\n";
  }
} // ProcessPortions

int main() {
  DefaultCoutFlags = std::cout.flags();
  std::cout << "Starting..." << std::endl;

  auto foods = GetFoods();
  ProcessNutrients(foods);
  ProcessPortions(foods);
  return 0;
} // main
