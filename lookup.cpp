#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <chrono>
#include <limits>
#include <cstdlib>

const std::string UsdaPath = "c:/Users/tjgolubi/prj/usda/";
const std::string FdcPath  = UsdaPath + "fdc/";
const std::string SrPath   = UsdaPath + "sr/";

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
auto Parse(const std::string& str, char sep=',', char delim='"', char
escape='\\')
  -> ParseVec<E>
{
  std::istringstream iss(str);
  std::string s;
  ParseVec<E> rval;
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
      rval.push_back(s);
    char c;
    while (iss >> c) {
      if (c != sep)
        break;
      if (iss.peek() == delim) {
	iss >> std::quoted(s, delim, escape);
      }
      else {
        std::getline(iss, s, sep);
	if (!iss.eof())
	  iss.unget();
      }
      if (iss)
	rval.push_back(s);
    }
  }
  return rval;
} // Parse

template<class E>
auto ParseTxt(const std::string& str) { return Parse<E>(str, '^', '~'); }

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

auto AtwaterString(const std::string& prot, const std::string& fat, const
std::string& carb)
  -> std::string
{
  auto dashed_null = [](const std::string& s) {
    static const std::string dash("-");
    return s.empty() ? dash : s;
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
    enum class Fncf { id, protein, fat, carb };
    auto fname = FdcPath + "food_calorie_conversion_factor.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    if (!std::getline(input, line)) // discard header
      return atwaterFoods;
    while (std::getline(input, line)) {
      auto v = Parse<Fncf>(line);
      const auto& id = v[Fncf::id];
      auto atwater =
	  AtwaterString(v[Fncf::protein], v[Fncf::fat], v[Fncf::carb]);
      atwaterCodes.emplace(id, atwaterDb.get(atwater));
    }
    std::cout << "Read " << atwaterCodes.size() << " Atwater codes ("
	<< atwaterDb.size() << " unique).\n";
  }
  {
    enum Fncf { id, fdc_id };
    auto fname = FdcPath + "food_nutrient_conversion_factor.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    std::getline(input, line); // discard header
    while (std::getline(input, line)) {
      auto v = Parse<Fncf>(line);
      auto iter = atwaterCodes.find(v[Fncf::id]);
      if (iter != atwaterCodes.end())
	atwaterFoods[v[Fncf::fdc_id]] = iter->second;
    }
  }
  return atwaterFoods;
} // ReadAtwaterFoods

struct FormatSaver {
  std::ostream& os;
  std::ios::fmtflags flags;
  std::streamsize prec;
  std::streamsize width;
  explicit FormatSaver(std::ostream& os_)
    : os{os_}, flags{os_.flags()}, prec{os_.precision()}, width{os_.width()}
    { }
  ~FormatSaver() {
    os.flags(flags);
    os.precision(prec);
    os.width(width);
  }
}; // FormatSaver

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
  FormatSaver saver(os);
  using namespace std;
  os << fixed;
  os.precision(2);
  os << fixed << setw(7) << f.energy()
     <<   ' ' << setw(7) << f.protein()
     <<   ' ' << setw(7) << f.fat()
     <<   ' ' << setw(7) << f.carb()
     <<   ' ' << setw(7) << fiber;
  return os << ' ' << f.desc;
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
    enum class Legacy { fc_id, NDB_number };
    auto fname = FdcPath + "sr_legacy_food.csv";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    std::getline(input, line);  // discard headings
    while (std::getline(input, line)) {
      auto v = Parse<Legacy>(line);
      if (foods.contains(v[Legacy::fc_id]))
        legacy.emplace(v[Legacy::NDB_number], v[Legacy::fc_id]);
    }
  }
  std::cout << "Found " << legacy.size() << " legacy foods.\n";
  {
    enum class FoodDesc {
      NDB_No, FdGrp_Cd, Long_Desc, Shrt_Desc, ComName, ManufacName, Survey,
      Ref_desc, Refuse, SciName, N_Factor, Pro_Factor, Fat_Factor, CHO_Factor
    }; // FoodDesc
    auto fname = SrPath + "FOOD_DES.txt";
    auto input = std::ifstream(fname);
    if (!input)
      throw std::runtime_error("Cannot open " + fname);
    std::getline(input, line);  // discard headings
    int updateCount = 0;
    int linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      try {
	auto v = ParseTxt<FoodDesc>(line);
	auto iter = legacy.find(v[FoodDesc::NDB_No]);
	if (iter == legacy.end())
	  continue;
	auto& ingred = foods.at(iter->second);
	if (ingred.atwater != 0)
	  continue;
	auto str = AtwaterString(v[FoodDesc::Pro_Factor],
	                         v[FoodDesc::Fat_Factor],
				 v[FoodDesc::CHO_Factor]);
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

int main() {
  std::cout << "Starting..." << std::endl;
  StringDb atwaterDb;

  auto atwaterFoods = ReadAtwaterFoods(atwaterDb);

  auto foods = ReadFoods(atwaterFoods);

  UpdateAtwaterFromLegacy(foods, atwaterDb);

  enum class FoodNutrient {
    id, fdc_id, nutrient_id, amount, data_points, derivation_id,
    min, max, median, log, footnote, min_year_acquired
  }; // FoodNutrient

  std::string fname = FdcPath + "food_nutrient.csv";
  auto db = std::ifstream{fname};
  if (!db)
    throw std::runtime_error("Cannot open " + fname);
  long long linenum = 0;
  constexpr auto total_lines = 26235968;
  auto t = std::chrono::steady_clock::now();
  std::string id, fdc_id, nutrient_id, amount;
  constexpr auto max_size = std::numeric_limits<std::streamsize>::max();
  db.ignore(max_size, '\n');
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
  std::multimap<int, const Ingred*> group;
  for (const auto& [id, ingred]: foods)
    group.emplace(ingred.atwater, &ingred);
  std::cout << std::fixed << std::setprecision(2);
  int last_atwater = 0;
  for (const auto& [id, ingred]: group) {
    if (id != last_atwater) {
      last_atwater = id;
      std::cout << '[' << atwaterDb.str(id) << "]\n";
    }
    std::cout << "    100     0 " << (*ingred) << " // " << ingred->id << '\n';
  }
  return 0;
} // main
