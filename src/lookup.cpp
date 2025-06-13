// Copyright 2023-2024 Terry Golubiewski, all rights reserved.

#include "Atwater.h"
#include "To.h"
#include "Parse.h"

#include <mp-units/systems/si.h>
#include <mp-units/systems/usc.h>
#include <mp-units/math.h>
#include <mp-units/ostream.h>

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

const auto DbPath = std::string{std::getenv("FOOD_PATH")} + "/";

constexpr auto Round(float x) -> float
  { return (std::abs(x) < 10) ? (std::round(10 * x) / 10) : std::round(x); }

template<auto U, typename R>
constexpr bool IsZero(const mp_units::quantity<U, R>& q)
{ return (q == q.zero()); }

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

namespace my {

inline constexpr struct Calorie final
  : mp_units::named_unit<"cal",
                         mp_units::mag_ratio<4184, 1000> * mp_units::si::joule>
  {} Calorie;
inline constexpr auto kiloCalorie = mp_units::si::kilo<Calorie>;
inline constexpr auto Kcal = kiloCalorie;

} // my

struct Ingred {
  using Energy = mp_units::quantity<my::Kcal, float>;
  using Weight = mp_units::quantity<mp_units::si::gram  , float>;
  FdcId id;
  std::string desc;
  Energy kcal;
  Weight protein;
  Weight fat;
  Weight carb;
  Weight fiber;
  Weight alcohol;
  Atwater atwater;
  friend auto operator<=>(const Ingred&, const Ingred&) = default;
}; // Ingred

std::ostream& operator<<(std::ostream& os, const Ingred& f) {
  std::ostringstream ostr;
  using namespace std;
  using namespace mp_units::si;
  ostr << setw(5) << Round(f.kcal.numerical_value_in(Kcal))
       << fixed << setprecision(2)
       << ' ' << setw(6) << f.protein.numerical_value_in(gram)
       << ' ' << setw(6) << f.fat.numerical_value_in(gram)
       << ' ' << setw(6) << f.carb.numerical_value_in(gram)
       << ' ' << setw(6) << f.fiber.numerical_value_in(gram)
       << ' ' << setw(6) << f.alcohol.numerical_value_in(gram)
       << ' ' << f.atwater
       << ' ' << f.desc;
  return os << ostr.str();
} // << Ingred

class OutIngred {
  const Ingred& ingred;
public:
  explicit OutIngred(const Ingred& ing_) : ingred{ing_} {
    if (!IsZero(ingred.alcohol) && !IsZero(ingred.fiber)) {
      throw std::runtime_error{
	  std::to_string(ingred.id) + " invalid alcohol/fiber"};
    }
  }
  friend std::ostream& operator<<(std::ostream& os, const OutIngred& out) {
    std::ostringstream ostr;
    using namespace std;
    using namespace mp_units::si;
    const auto& f = out.ingred;
    auto x = IsZero(f.alcohol) ? f.fiber : -f.alcohol;
    ostr << setw(5) << Round(f.kcal.numerical_value_in(Kcal))
	 << fixed << setprecision(2)
	 << ' ' << setw(6) << f.protein.numerical_value_in(gram)
	 << ' ' << setw(6) << f.fat.numerical_value_in(gram)
	 << ' ' << setw(6) << f.carb.numerical_value_in(gram)
	 << ' ' << setw(6) << x.numerical_value_in(gram)
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
      using namespace mp_units::si;
      ingred->id      = fdc_id;
      ingred->kcal    = To<float>(v[Idx::kcal])  * Kcal;
      ingred->protein = To<float>(v[Idx::prot])  * gram;
      ingred->fat     = To<float>(v[Idx::fat])   * gram;
      ingred->carb    = To<float>(v[Idx::carb])  * gram;
      ingred->fiber   = To<float>(v[Idx::fiber]) * gram;
      ingred->alcohol = To<float>(v[Idx::alc])   * gram;
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
  using Wt  = mp_units::quantity<mp_units::si::gram, float>;
  using Vol = mp_units::quantity<mp_units::si::millilitre, float>;
  FdcId id;
  Wt  wt;
  Vol vol;
  std::string desc;
  std::string comment;
  Portion(FdcId id_, Wt wt_, Vol vol_,
	  std::string desc_, std::string comment_)
    : id{id_}, wt{wt_}, vol{vol_}
    , desc{std::move(desc_)}, comment{std::move(comment_)}
    { }
  friend auto operator<=>(const Portion&, const Portion&) = default;
}; // Portion

std::ostream& operator<<(std::ostream& os, const Portion& p) {
  os << '{' << p.id << ' ' << p.wt << ' ' << p.vol << ' ' << p.desc << '}';
  return os;
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
      using namespace mp_units::si;
      ++linenum;
      ParseTsv(v, line);
      auto fdc_id = FdcId{To<int>(v[Idx::fdc_id])};
      if (!rng::binary_search(fdc_ids, fdc_id))
	continue;
      rval.emplace_back(fdc_id,
                        To<float>(v[Idx::g])  * gram,
                        To<float>(v[Idx::ml]) * milliliter,
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
  using Vol = mp_units::quantity<mp_units::si::millilitre, float>;
  static constexpr auto Round(Vol x) {
    using namespace mp_units::si;
    auto v = x.numerical_value_in(millilitre);
    v = (std::abs(v) >= 100.0f) ? (std::round(v * 10) / 10)
				: (std::round(v * 100) / 100);
    return v * millilitre;
  }
  std::map<Vol, std::string> dict;
public:
  MlText() {
    constexpr auto FlOz   = 1.0f * mp_units::usc::fluid_ounce;
    constexpr auto Cup    = 1.0f * mp_units::usc::cup;
    constexpr auto Tbsp   = 1.0f * mp_units::usc::tablespoon;
    constexpr auto Tsp    = 1.0f * mp_units::usc::teaspoon;
    constexpr auto Pint   = 1.0f * mp_units::usc::pint;
    constexpr auto Quart  = 1.0f * mp_units::usc::quart;
    constexpr auto Gallon = 1.0f * mp_units::usc::gallon;
#if 0
    constexpr auto Cup    = 8 * FlOz;
    constexpr auto Tbsp   = Cup / 16;
    constexpr auto Tsp    = Tbsp / 3;
    constexpr auto Pint   = 2 * Cup;
    constexpr auto Quart  = 4 * Cup;
    constexpr auto Gallon = 4 * Quart;
#endif
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
  std::string operator()(Vol x) const {
    x = Round(x);
    if (auto iter = dict.find(x); iter != dict.end())
      return iter->second;
    if (mp_units::abs(x) < 0.05f * mp_units::si::millilitre)
      return "0";
    std::ostringstream oss;
    oss << x;
    return oss.str();
  }
}; // MlText

int main() {
  using namespace std::literals;

  try {
    DefaultCoutFlags = std::cout.flags();

    auto foods = ReadFoods();

    LoadNutrients(foods);

    const auto portions = LoadPortions(foods);

    const auto fname = "lookout.nut"s;
    auto output = std::ofstream(fname);
    output << "#include \"defs.nut\"\n";
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
	auto r =
	    rng::equal_range(portions, ingred.id, rng::less{}, &Portion::id);

	using std::setw, std::left, std::right, std::quoted;

	std::ostringstream ostr;
	for (const auto& p: r) {
          using namespace mp_units::si;
	  ostr << (IsZero(p.vol) ? '*' : ' ')
               << setw(5) << Round(p.wt.numerical_value_in(gram))
	       << ' ' << setw(5) << mlStr(p.vol)
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
  }
  catch (const std::ios::failure& fail) {
    std::cout << "ios::failure: " << fail.what()
	<< "\n    error code = " << fail.code().message() << std::endl;

  }
  catch (const std::exception& x) {
    std::cout << "standard exception: " << x.what() << std::endl;
  }
  return EXIT_FAILURE;
} // main
