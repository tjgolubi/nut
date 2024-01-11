// Copyright 2023 Terry Golubiewski, all rights reserved.

#include "Atwater.h"

#include <system_error>
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

const std::string UsdaPath = "c:/Users/tjgolubi/prj/usda/";
const std::string FdcPath  = UsdaPath + "fdc/";
const std::string SrPath   = UsdaPath + "sr/";

std::ios::fmtflags DefaultCoutFlags;

constexpr auto Round(float x) -> float
  { return (std::abs(x) < 10) ? (std::round(10 * x) / 10) : std::round(x); }

template<class E>
struct ParseVec: public std::vector<std::string> {
  using base_type = std::vector<std::string>;
  const std::string& at(E e) const {
    auto idx = static_cast<base_type::size_type>(e);
    return base_type::at(idx);
  }
  const std::string& operator[](E e) const { return this->at(e); }
}; // ParseVec

template <class E>
std::ostream& operator<<(std::ostream& os, const ParseVec<E>& v) {
  os << '<';
  for (const auto& s: v)
    os << ' ' << std::quoted(s);
  return os << " >";
} // << ParseVec

template<class E>
auto Parse(ParseVec<E>& v, const std::string& str,
           char sep=',', char delim='"', char escape='\\')
  -> ParseVec<E>&
{
  std::istringstream iss(str);
  std::string s;
  v.clear();
  if (iss >> std::ws) {
    if (iss.peek() == delim) {
      iss >> std::quoted(s, delim, escape);
    }
    else {
      std::getline(iss, s, sep);
      if (!iss.eof())
	iss.unget();
    }
    if (iss)
      v.push_back(s);
    char c;
    while (iss >> c) {
      if (c != sep)
        break;
      if (iss.peek() == delim) {
	if (iss >> std::quoted(s, delim, escape))
	  v.push_back(s);
	continue;
      }
      std::getline(iss, s, sep);
      v.push_back(s);
      if (!iss.eof())
	iss.unget();
    }
  }
  return v;
} // Parse

template<class E>
auto ParseTxt(ParseVec<E>& v, const std::string& str) -> ParseVec<E>&
  { return Parse<E>(v, str, '^', '~'); }

#if 0
template<class E>
auto Parse(const std::string& str,
           char sep=',', char delim='"', char escape='\\')
  -> ParseVec<E>
{
  ParseVec<E> rval;
  ParseVec<E>(rval, str, sep, delim, escape);
  return rval;
} // Parse


template<class E>
auto ParseTxt(const std::string& str) { return Parse<E>(str, '^', '~'); }
#endif

class StringDb {
private:
  std::vector<std::string> strings;
  std::map<std::string, int> known;
public:
  int size() const { return strings.size(); }
  const std::string& str(int idx) const { return strings.at(idx); }
  int get(const std::string& str) {
    auto iter = known.find(str);
    if (iter != known.end())
      return iter->second;
    int idx = strings.size();
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

auto ToStr(float x) {
  std::array<char, 16> buf;
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), x,
				 std::chars_format::fixed, 2);
  if (ec != std::errc())
    throw std::system_error(std::make_error_code(ec));
  return std::string(buf.data(), ptr);
}

auto AtwaterString(const std::string& prot, const std::string& fat,
                   const std::string& carb)
  -> std::string
{
  auto dashed_null = [](const std::string& s) {
    return s.empty() ? "-" : ToStr(std::stof(s));
  }; // dashed_null
  if (prot.empty() && fat.empty() && carb.empty())
    return std::string("");
  return dashed_null(prot) + " " + dashed_null(fat) + " " + dashed_null(carb);
} // AtwaterString

auto ReadAtwaterFoods(StringDb& atwaterDb)
  -> std::map<std::string, int>
{
  std::map<std::string, int> atwaterFoods;
  std::map<std::string, int> atwaterCodes;
  std::string line;
  {
    enum class Idx { id, protein, fat, carb, end };
    auto fname = FdcPath + "food_calorie_conversion_factor.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    Parse(v, line);
    if (v.size() != std::size_t(Idx::end)
        || v[Idx::id]      != "food_nutrient_conversion_factor_id"
	|| v[Idx::protein] != "protein_value"
	|| v[Idx::fat]     != "fat_value"
	|| v[Idx::carb]    != "carbohydrate_value")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      Parse(v, line);
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
    auto fname = FdcPath + "food_nutrient_conversion_factor.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    Parse(v, line);
    if (v.size() != std::size_t(Idx::end)
	|| v[Idx::id]     != "id"
	|| v[Idx::fdc_id] != "fdc_id")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      Parse(v, line);
      auto iter = atwaterCodes.find(v[Idx::id]);
      if (iter != atwaterCodes.end())
	atwaterFoods[v[Idx::fdc_id]] = iter->second;
    }
  }
  return atwaterFoods;
} // ReadAtwaterFoods

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

struct Ingred {
  std::string id;
  std::string desc;
  FieldValues values;
  int atwater = 0;
  float value(FieldIdx field) const {
    auto idx = static_cast<FieldValues::size_type>(field);
    return values.at(idx);
  }
  Ingred() { values.fill(0.0); }
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
    constexpr auto kcalPerKj = 0.239;
    return kcalPerKj * value(FieldIdx::kJ);
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
  bool operator<(const Ingred& rhs) const { return (id < rhs.id); }
}; // Ingred

std::ostream& operator<<(std::ostream& os, const Ingred& f) {
  auto fiber = -f.alcohol();
  if (fiber == 0.0)
    fiber = f.fiber();
  std::ostringstream ostr;
  using namespace std;
  ostr << setw(5) << Round(f.energy())
       << fixed << setprecision(2)
       << ' ' << setw(6) << f.protein()
       << ' ' << setw(6) << f.fat()
       << ' ' << setw(6) << f.carb()
       << ' ' << setw(6) << fiber
       << ' ' << f.desc;
  return os << ostr.str();
} // << Ingred

auto ReadFoods(const std::map<std::string, int>& atwaterFoods)
  -> std::map<std::string, Ingred>
{
  std::map<std::string, Ingred> foods;
  auto input = std::ifstream{"lookup.txt"};
  if (!input)
    throw std::runtime_error("Cannot open lookup.txt");
  Ingred ingred;
  while (input >> ingred.id >> std::ws) {
    if (std::getline(input, ingred.desc)) {
      if (auto iter = atwaterFoods.find(ingred.id); iter != atwaterFoods.end())
        ingred.atwater = iter->second;
      else
        ingred.atwater = 0;
      foods.emplace(ingred.id, ingred);
    }
  }
  std::cout << "Read " << foods.size() << " foods.\n";
  return foods;
} // ReadFoods

void UpdateAtwaterFromLegacy(std::map<std::string, Ingred>& foods,
			     StringDb& atwaterDb)
{
  std::cout << "Reading legacy Atwater codes.\n";
  std::string line;
  std::map<std::string, std::string> legacy;
  {
    enum class Idx { fdc_id, NDB_number, end };
    auto fname = FdcPath + "sr_legacy_food.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    Parse(v, line);
    if (v.size() != std::size_t(Idx::end)
        || v[Idx::fdc_id]     != "fdc_id"
	|| v[Idx::NDB_number] != "NDB_number")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      Parse(v, line);
      if (foods.contains(v[Idx::fdc_id]))
        legacy.emplace(v[Idx::NDB_number], v[Idx::fdc_id]);
    }
  }
  std::cout << "Found " << legacy.size() << " legacy foods.\n";
  {
    enum class Idx {
      NDB_No, FdGrp_Cd, Long_Desc, Shrt_Desc, ComName, ManufacName, Survey,
      Ref_desc, Refuse, SciName, N_Factor, Pro_Factor, Fat_Factor, CHO_Factor,
      end
    };
    auto fname = SrPath + "FOOD_DES.txt";
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
	ParseTxt<Idx>(v, line);
	if (v.size() != std::size_t(Idx::end)) {
	  throw std::runtime_error(
	      "invalid # columns: " + std::to_string(v.size()));
	}
	auto iter = legacy.find(v[Idx::NDB_No]);
	if (iter == legacy.end())
	  continue;
	auto& ingred = foods.at(iter->second);
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

float ConversionFactor(const std::string& unit) {
  constexpr float Cup = 236.6;
  static const std::map<std::string, float> FactorMap = {
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
  if (unit.empty())
    return 0.0f;
  if (auto iter = FactorMap.find(unit); iter != FactorMap.end())
    return iter->second;
  return 0.0f;
} // ConversionFactor

struct Portion {
  const Ingred& ingred;
  float g  = 0.0;
  float ml = 0.0;
  std::string desc;
  Portion(const Ingred& i, const std::string& n, float g_, float ml_)
    : ingred{i}, desc(n), g{g_}, ml{ml_} { }
}; // Portion

std::ostream& operator<<(std::ostream& os, const Portion& p) {
  return os << '{' << p.g << ' ' << p.ml << ' ' << p.desc << '}';
} // << Portion

auto LoadPortions(const std::map<std::string, Ingred>& foods)
  -> std::multimap<std::string, Portion>
{
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
    const auto fname = FdcPath + "measure_unit.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line)) // discard headings
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    Parse(v, line);
    if (v.size() != std::size_t(Idx::end)
      || v[Idx::id]   != "id"
      || v[Idx::name] != "name")
    throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      Parse(v, line);
      if (v[Idx::id] != "9999")
	units.emplace(v[Idx::id], Unit(v[Idx::name]));
    }
    std::cout << "Loaded " << units.size() << " units of measure.\n";
  }
  std::multimap<std::string, Portion> rval;
  {
    enum class Idx { id, fdc_id, seq_num, amount, unit, desc,
	modifier, grams, data_points, footnote, min_year_acquired, end };
    const auto fname = FdcPath + "food_portion.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line)) // discard headings
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    Parse(v, line);
    if (v.size() != std::size_t(Idx::end)
	|| v[Idx::fdc_id]   != "fdc_id"
	|| v[Idx::grams]    != "gram_weight"
	|| v[Idx::amount]   != "amount"
	|| v[Idx::unit]     != "measure_unit_id"
	|| v[Idx::desc]     != "portion_description"
	|| v[Idx::modifier] != "modifier")
      throw std::runtime_error(fname + ": invalid headings");
    while (std::getline(input, line)) {
      Parse(v, line);
      const auto& fdc_id = v[Idx::fdc_id];
      auto food = foods.find(fdc_id);
      if (food == foods.end())
        continue;
      const Ingred& ingred = food->second;
      float ml = 0.0f;
      float g  = std::stof(v[Idx::grams]);
      std::ostringstream desc;
      const auto& amount = v[Idx::amount];
      float val = amount.empty() ? 0.0 : std::stof(amount);
      auto iter = units.find(v[Idx::unit]);
      const auto& unit = (iter != units.end()) ? iter->second : NullUnit;
      if (!unit.name.empty()) {
        desc << unit.name;
	ml = val * unit.ml_factor;
	val = 0.0;
      }
      auto modifier{v[Idx::modifier]};
      if (val != 0.0f && !modifier.empty()) {
        auto pos = modifier.find(',');
	ml = val * ConversionFactor(modifier.substr(0, pos));
	if (ml != 0.0f) {
	  val = 0.0;
	  constexpr auto npos = std::string::npos;
	  modifier = (pos != npos && pos+1 < v[Idx::modifier].size())
	           ? v[Idx::modifier].substr(pos+1)
	           : std::string{};
	  if (!modifier.empty()) {
	    pos = modifier.find_first_not_of(" ");
	    modifier = (pos != npos) ? modifier.substr(pos) : std::string{};
	  }
	}
      }
      if (val != 0.0 && val != 1.0)
        desc << val << 'x';
      if (!v[Idx::desc].empty()) {
        if (!desc.str().empty())
	  desc << ' ';
	desc << v[Idx::desc];
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
        if (!desc.str().empty())
	  desc << ' ';
	desc << modifier;
      }
      if (!desc.str().empty())
        desc << ' ';
      desc << "$this";
      if (!comment.empty())
        desc << " // " << comment;
      rval.emplace(fdc_id, Portion{ingred, desc.str(), g, ml});
    }
  }
  std::cout << "Loaded " << rval.size() << " portions.\n";
  return rval;
} // LoadPortions

auto MakeAtwaterNames() {
  std::map<std::string, std::string> rval;
  for (const auto& [name, atwater]: Atwater::Names) {
    auto str =       ToStr(atwater.prot)
             + " " + ToStr(atwater.fat)
             + " " + ToStr(atwater.carb);
    rval.emplace(str, name);
  }
  return rval;
}

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
  DefaultCoutFlags = std::cout.flags();
  std::cout << "Starting..." << std::endl;
  StringDb atwaterDb;

  auto atwaterFoods = ReadAtwaterFoods(atwaterDb);

  auto foods = ReadFoods(atwaterFoods);

  UpdateAtwaterFromLegacy(foods, atwaterDb);

  enum class Idx {
    id, fdc_id, nutrient_id, amount, data_points, derivation_id,
    min, max, median, log, footnote, min_year_acquired, end
  }; // Idx

  std::string fname = FdcPath + "food_nutrient.csv";
  auto db = std::ifstream{fname};
  if (!db)
    throw std::runtime_error("Cannot open " + fname);
  {
    std::string line;
    if (!std::getline(db, line))
      throw std::runtime_error("Cannot read " + fname);
    ParseVec<Idx> v;
    Parse(v, line);
    if (v.size() != std::size_t(Idx::end)
        || v[Idx::id]          != "id"
	|| v[Idx::fdc_id]      != "fdc_id"
	|| v[Idx::nutrient_id] != "nutrient_id"
	|| v[Idx::amount]      != "amount")
      throw std::runtime_error(fname + ": invalid headings");
  }
  long long linenum = 0;
  constexpr auto total_lines = 26235968;
  auto t = std::chrono::steady_clock::now();
  std::string id, fdc_id, nutrient_id, amount;
  constexpr auto max_size = std::numeric_limits<std::streamsize>::max();
  char c1, c2, c3;
  while (db >> std::quoted(id) >> c1 >> std::quoted(fdc_id) >> c2
	    >> std::quoted(nutrient_id) >> c3 >> std::quoted(amount))
  {
    db.ignore(max_size, '\n');
    ++linenum;
    if (std::chrono::steady_clock::now() > t) {
      t += std::chrono::seconds(1);
      auto percent = (100 * linenum) / total_lines;
      std::cout << '\r' << percent << "% complete" << std::flush;
    }
    if (c1 != ',' || c2 != ',' || c3 != ',')
      continue;
    auto food = foods.find(fdc_id);
    if (food == foods.end())
      continue;
    auto iter = std::find(FieldIds.begin(), FieldIds.end(), nutrient_id);
    if (iter == FieldIds.end())
      continue;
    auto i = std::distance(FieldIds.begin(), iter);
    food->second.values.at(i) = std::stof(amount);
  }
  std::cout << "\r100% complete\n";
  auto output = std::ofstream("lookout.txt");
  if (!output)
    throw std::runtime_error("Could not write lookout.txt");
  const MlText mlStr;
  const auto portions = LoadPortions(foods);
  auto atwaterNames = MakeAtwaterNames();
  std::vector<const Ingred*> v;
  for (const auto& [id, ingred]: foods)
    v.push_back(&ingred);
  auto lt = [atwaterDb](const Ingred* lhs, const Ingred* rhs) {
    if (auto cmp =
		  (atwaterDb.str(lhs->atwater) <=> atwaterDb.str(rhs->atwater));
	cmp != 0)
      return (cmp < 0);
    return (lhs->desc < rhs->desc);
  };
  std::sort(v.begin(), v.end(), lt);
  output << std::fixed << std::setprecision(2);
  int last_atwater = 0;
  for (const auto& ptr: v) {
    if (ptr->atwater != last_atwater) {
      last_atwater = ptr->atwater;
      auto str = atwaterDb.str(ptr->atwater);
      if (auto iter = atwaterNames.find(str); iter != atwaterNames.end())
        str = iter->second;
      output << '[' << str << "]\n";
    }
    output << "   100     0 " << (*ptr)
              << " // usda " << ptr->id << '\n';
    {
      auto r = portions.equal_range(ptr->id);
      std::vector<const Portion*> v2;
      for (auto it = r.first; it != r.second; ++it)
        v2.push_back(&it->second);
      auto lt2 = [](const Portion* lhs, const Portion* rhs) {
        if (auto cmp = (lhs->g <=> rhs->g); cmp != 0)
	  return (cmp < 0);
	if (auto cmp = (lhs->ml <=> rhs->ml); cmp != 0)
	  return (cmp < 0);
	return (lhs->desc < rhs->desc);
      };
      std::sort(v2.begin(), v2.end(), lt2);

      using std::setw, std::left, std::right, std::quoted;

      std::ostringstream ostr;
      for (const auto p: v2) {
	ostr << ((p->ml == 0.0f) ? '*' : ' ')<< setw(5) << Round(p->g)
	     << ' ' << setw(5) << mlStr(p->ml)
	     << ' ' << setw(5) << 0 // Round(p.ingred.energy() * p.g / 100)
	     << ' ' << left << setw(27) << "this"
	     << right << ' ' << p->desc << '\n';
      }
      output << ostr.str();
    }
  }
  return 0;
} // main
