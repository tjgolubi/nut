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
#include <limits>
#include <cstdlib>
#include <cmath>

const std::string UsdaPath = "c:/Users/tjgolubi/prj/usda/";
const std::string FdcPath  = UsdaPath + "fdc/";
const std::string SrPath   = UsdaPath + "sr/";

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

int main() {
  std::cerr << "Starting..." << std::endl;
  std::string line;
  enum class Idx { fdc_id, data_type, desc, category, pub_date, end };
  const auto fname = FdcPath + "food.csv";
  auto input = std::ifstream(fname);
  if (!input)
    throw std::runtime_error("Cannot open " + fname);
  if (!std::getline(input, line))
    throw std::runtime_error("Cannot read " + fname);
  ParseVec<Idx> v;
  Parse(v, line);
  if (v.size() != std::size_t(Idx::end)
      || v[Idx::fdc_id] != "fdc_id"
      || v[Idx::desc]   != "description")
    throw std::runtime_error("Invalid column headings");
  auto output = std::ofstream("food.txt");
  if (!output)
    throw std::runtime_error("Cannot open food.txt");
  long long linenum = 1;
  constexpr auto total_lines = 2021092;
  auto t = std::chrono::steady_clock::now();
  while (std::getline(input, line)) {
    ++linenum;
    if (std::chrono::steady_clock::now() > t) {
      t += std::chrono::seconds(1);
      auto percent = (100 * linenum) / total_lines;
      std::cerr << '\r' << percent << "% complete" << std::flush;
    }
    try {
      Parse(v, line);
      if (v.size() != std::size_t(Idx::end))
        throw std::range_error("Invalid # records read: "
				+ std::to_string(v.size()));
      const auto& data_type = v[Idx::data_type];
      if (data_type != "foundation_food" && data_type != "sr_legacy_food")
	continue;
      output << v[Idx::fdc_id] << "\t|" << v[Idx::desc] << '\n';
    }
    catch (const std::exception& x) {
      std::cerr << '\r' << fname << '(' << linenum << ") " << x.what() << '\n';
      std::cerr << line << '\n';
    }
  }
  std::cerr << "\r100% complete\n";
  return 0;
} // main
