#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <chrono>
#include <regex>
#include <limits>
#include <cstdlib>

template<class E>
struct ParseVec: public std::vector<std::string> {
  using base_type = std::vector<std::string>;
  const std::string& at(E e) const {
    auto idx = static_cast<base_type::size_type>(e);
    return base_type::at(idx);
  }
}; // ParseVec

template <class E>
std::ostream& operator<<(std::ostream& os, const ParseVec<E>& v) {
  os << '<';
  for (const auto& s: v)
    os << ' ' << s;
  return os << " >";
} // << ParseVec

template<class E>
auto Parse(const std::string& str)
  -> ParseVec<E>
{
  std::istringstream iss(str);
  std::string s;
  ParseVec<E> rval;
  if (iss >> std::ws >> std::quoted(s)) {
    rval.emplace_back(std::move(s));
    char c;
    while (iss >> std::ws >> c >> std::ws >> std::quoted(s)) {
      if (c != ',')
        break;
      rval.emplace_back(std::move(s));
    }
  }
  return rval;
} // Parse

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

auto ReadAtwaterFoods(StringDb& atwaterDb)
  -> std::map<std::string, int>
{
  std::map<std::string, int> atwaterFoods;
  std::map<std::string, int> atwaterCodes;
  std::string line;
  {
    enum class Fncf { id, protein, fat, carb };
    auto input = std::ifstream("usda/food_calorie_conversion_factor.csv");
    if (!std::getline(input, line)) // discard header
      return atwaterFoods;
    while (std::getline(input, line)) {
      auto v = Parse<Fncf>(line);
      const auto& id = v.at(Fncf::id);
      auto dashed_null = [](const std::string& s) -> const std::string& {
        static const std::string& dash("-");
        if (s.empty())
	  return dash;
	return s;
      }; // dashed_null
      auto atwater = dashed_null(v.at(Fncf::protein)) + " " +
	  dashed_null(v.at(Fncf::fat)) + " " + dashed_null(v.at(Fncf::carb));
      atwaterCodes.emplace(id, atwaterDb.get(atwater));
    }
    std::cout << "Read " << atwaterCodes.size() << " Atwater codes ("
	<< atwaterDb.size() << " unique).\n";
  }
  {
    enum Fncf { id, fdc_id };
    auto input = std::ifstream("usda/food_nutrient_conversion_factor.csv");
    std::getline(input, line); // discard header
    while (std::getline(input, line)) {
      auto v = Parse<Fncf>(line);
      auto iter = atwaterCodes.find(v.at(Fncf::id));
      if (iter != atwaterCodes.end())
	atwaterFoods[v.at(Fncf::fdc_id)] = iter->second;
    }
  }
  return atwaterFoods;
} // ReadAtwaterFoods

enum class FieldIdx {
  energy, atwater_general, atwater_specific, protein, fat, carb_diff, carb_sum,
  fiber, alcohol, end
}; // FieldIdx

const std::vector<std::string> FieldIds = {
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
    return value(FieldIdx::energy);
  }
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
  os << f.energy() << ' ' << f.protein() << ' ' << f.fat()
      << ' ' << f.carb() << ' ' << f.fiber() << ' ' << f.alcohol()
      << ' ' << f.atwater
      << ' ' << f.desc;
  return os;
}

auto ReadFoods(const std::map<std::string, int>& atwaterFoods,
               std::multimap<int, std::string>& atwaterGroups)
  -> std::map<std::string, Ingred>
{
  std::map<std::string, Ingred> foods;
  auto input = std::ifstream{"lookup.txt"};
  Ingred ingred;
  while (input >> ingred.id >> std::ws) {
    if (std::getline(input, ingred.desc)) {
      if (auto iter = atwaterFoods.find(ingred.id); iter != atwaterFoods.end())
        ingred.atwater = iter->second;
      else
        ingred.atwater = 0;
      foods.emplace(ingred.id, ingred);
      atwaterGroups.emplace(ingred.atwater, ingred.id);
    }
  }
  std::cout << "Read " << foods.size() << " foods.\n";
  return foods;
} // ReadFoods

int main() {
  std::cout << "Starting..." << std::endl;
  StringDb atwaterDb;

  auto atwaterFoods = ReadAtwaterFoods(atwaterDb);

  std::multimap<int, std::string> atwaterGroups;
  auto foods = ReadFoods(atwaterFoods, atwaterGroups);

  enum class FoodNutrient {
    id, fdc_id, nutrient_id, amount, data_points, derivation_id,
    min, max, median, log, footnote, min_year_acquired
  }; // FoodNutrient

  auto db = std::ifstream{"usda/food_nutrient.csv"};
  long long linenum = 0;
  constexpr auto total_lines = 26235968;
  auto t = std::chrono::steady_clock::now();
  std::string line;
  std::smatch m;
  std::getline(db, line); // discard heading
  while (std::getline(db, line)) {
    ++linenum;
    if (std::chrono::steady_clock::now() > t) {
      t += std::chrono::seconds(1);
      auto percent = (100 * linenum) / total_lines;
      std::cout << '\r' << percent << "% complete" << std::flush;
    }
    static const
	std::regex e{"^\"[^\"]*\",\"([^\"]*)\",\"([^\"]*)\",\"([^\"]*)\""};
    // auto v = Parse<FoodNutrient>(line);
    std::regex_search(line, m, e);
    if (m.size() != 4)
      continue;
    auto food = foods.find(m[1]);
    if (food == foods.end())
      continue;
    auto iter = std::find(FieldIds.begin(), FieldIds.end(), m[2]);
    if (iter == FieldIds.end())
      continue;
    auto i = std::distance(FieldIds.begin(), iter);
    food->second.values.at(i) = std::stof(m[3]);
  }
  std::cout << "\r100% complete\n";
  int last_group = 0;
  for (const auto& [group, id]: atwaterGroups) {
    if (group != last_group) {
      last_group = group;
      std::cout << '[' << atwaterDb.str(group) << "]\n";
    }
    std::cout << foods.at(id) << '\n';
  }
  return 0;
} // main
