
// Copyright 2024 Terry Golubiewski, all rights reserved.

#include "To.h"
#include "Parse.h"
#include "Progress.h"

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

const std::string UsdaPath = "../../usda/";
const std::string FnddsPath  = UsdaPath + "fndds/";

std::ios::fmtflags DefaultCoutFlags;

constexpr auto Round(float x) -> float
  { return (std::abs(x) < 10) ? (std::round(10 * x) / 10) : std::round(x); }

class FdcId {
private:
  gsl::index idx = 0;
public:
  FdcId() : idx{} { }
  explicit FdcId(int id) : idx(id) { }
  operator gsl::index() const { return idx; }
}; // FdcId

struct Ingred {
  FdcId id;
  std::string desc;
  float kcal    = 0.0f;
  float protein = 0.0f;
  float fat     = 0.0f;
  float carb    = 0.0f;
  float fiber   = 0.0f;
  float alcohol = 0.0f;
  Ingred() = default;
  Ingred(FdcId id_, std::string desc_) : id{id_}, desc{desc_} { }
  Ingred(FdcId id_, std::string_view desc_) : id{id_}, desc{desc_} { }
  auto operator<=>(const Ingred& rhs) const = default;
}; // Ingred

void Update(Ingred& ingred, const auto& code, const auto& value) {
  if (code.empty() || value.empty())
    return;

  static const std::array<std::string_view, 6> codes = {
    "203", // protein
    "204", // fat
    "205", // carb
    "208", // kcal
    "221", // alcohol
    "291"  // fiber
  }; // codes
  auto iter = rng::lower_bound(codes, code);
  if (iter == codes.end() || *iter != code)
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
      throw std::out_of_range("Update ingredient: code");
  }
} // Update ingred

std::ostream& operator<<(std::ostream& os, const Ingred& f) {
  std::ostringstream ostr;
  using namespace std;
  ostr << fixed << setprecision(2)
              << setw(6) << f.id
       << ' ' << setw(6) << f.kcal
       << ' ' << setw(6) << f.protein
       << ' ' << setw(6) << f.fat
       << ' ' << setw(6) << f.carb
       << ' ' << setw(6) << f.fiber
       << ' ' << setw(6) << f.alcohol
       << ' ' << f.desc;
  return os << ostr.str();
} // << Ingred

auto GetFoods() -> std::vector<Ingred> {
  auto outname = std::string("food.txt");
  auto output = std::ofstream(outname);
  if (!output)
    throw std::runtime_error("Cannot write " + outname);
  std::string line;
  const auto fname = FnddsPath + "mainfooddesc.tsv";
  auto input = std::ifstream(fname);
  if (!input)
    throw std::runtime_error("Cannot open " + fname);

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
    throw std::runtime_error("Cannot read " + fname);
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  std::vector<Ingred> rval;
  std::cout << "Reading " << fname << '\n';
  {
    auto progress = ProgressMonitor{5625};
    long long linenum = 1;
    while (std::getline(input, line)) {
      ++linenum;
      progress(linenum);
      try {
	ParseTsv(v, line);
	rval.emplace_back(FdcId{To<int>(v[Idx::fdc_id])}, v[Idx::desc]);
	output << v[Idx::fdc_id] << "\t|" << v[Idx::desc] << '\n';
      }
      catch (const std::exception& x) {
	std::cerr << '\r' << fname << '(' << linenum << ") " << x.what() << '\n';
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
    fdc_id, code, amount, start_date, end_date, end
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
    throw std::runtime_error("Cannot open " + fname);
  std::string line;
  if (!std::getline(input, line))
    throw std::runtime_error("Cannot read " + fname);
  ParseVec<Idx> v;
  ParseTsv(v, line);
  CheckHeadings(v, headings);
  std::cout << "Reading " << fname << '\n';
  {
    auto progress = ProgressMonitor{365561};
    long long linenum = 1;
    FdcId last_id;
    Ingred* last_ingred = nullptr;
    while (std::getline(input, line)) {
      ++linenum;
      progress(linenum);
      try {
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
	Update(*ingred, v[Idx::code], v[Idx::amount]);
      }
      catch (const std::exception& x) {
	std::cerr << '\r' << fname << '(' << linenum << ") " << x.what() << '\n';
	std::cerr << line << '\n';
      }
    }
  }
  const std::string outname{"fndds_foods.tsv"};
  auto output = std::ofstream(outname);
  if (!output)
    throw std::runtime_error("Could not write " + outname);
  output << "fdc_id\tkcal\tprot\tfat\tcarb\tfiber\talc\tdesc\n";
  output << std::fixed << std::setprecision(2);
  for (const auto& ingred: foods) {
    output << ingred.id
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

int main() {
  DefaultCoutFlags = std::cout.flags();
  std::cout << "Starting..." << std::endl;
  auto foods = GetFoods();
  ProcessNutrients(foods);
  return EXIT_SUCCESS;
} // main
