// Copyright 2024 Terry Golubiewski, all rights reserved.

#include "Parse.h"
#include "To.h"

#include <gsl/gsl>

#include <regex>
#include <ranges>
#include <algorithm>
#include <string>
#include <string_view>
#include <map>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <cmath>
#include <cstring>
#include <cctype>
#include <cstdlib>

namespace rng = std::ranges;

int main() {
  const auto UsdaPath  = std::string{"../usda/"};
  const auto FnddsPath = UsdaPath + "fndds/";
  const auto ifname = FnddsPath + "foodportiondesc.tsv";
  auto input = std::ifstream{ifname};
  if (!input)
    throw std::runtime_error{"Cannot read " + ifname};

  enum class Idx { code, desc, start_date, end_date, end };

  static const std::array<std::string_view, int(Idx::end)> Headings = {
    "Portion_code",
    "Portion_description",
    "Start_date",
    "End_date"
  }; // Headings

  std::string line;
  ParseVec<Idx> v;
  if (!std::getline(input, line))
    throw std::runtime_error{"Cannot read " + ifname};
  ParseTsv(v, line);
  CheckHeadings(v, Headings);
  auto desc = std::string{};
  long long linenum = 1;
  while (std::getline(input, line)) {
    ++linenum;
    try {
      ParseTsv(v, line);
      {
        const auto sv = v[Idx::desc];
        desc.clear();
	desc.reserve(sv.size());
	auto to_lower = [](unsigned char c) -> unsigned char
	  { return std::tolower(c); };
	rng::transform(sv, std::back_inserter(desc), to_lower);
      }
      auto value = 1.0f;
      std::smatch m;
      static const auto re_float = std::regex{"^\\s*\\d+(\\.\\d*)?"};
      static const auto re_frac  =
				std::regex{"^\\s*(?:(\\d+)[ -])?(\\d+)/(\\d+)"};

      auto pos = std::size_t{};
      if (std::regex_search(desc, m, re_float)) {
        value = std::stof(desc, &pos);
      }
      else if(std::regex_search(desc, m, re_frac)) {
        auto base = std::string_view{m[1].first, m[1].second};
	auto num  = std::string_view{m[2].first, m[2].second};
	auto den  = std::string_view{m[3].first, m[3].second};
	value = To<float>(base) + To<float>(num) / To<float>(den);
	pos = std::distance(m[0].first, m[0].second);
      }
      if (value == 0.0f)
        continue;
      pos = desc.find_first_not_of(" \t\n\r\f\v", pos);
      if (pos == std::string::npos)
        continue;
      desc.erase(0, pos);

      constexpr float CUP = 236.59f;

      static const std::vector<std::pair<std::regex, float>> volumes {
	{ std::regex{"^cc\\b"},                    1  },
	{ std::regex{"^cubic[ -]centimeters?\\b"}, 1  },
	{ std::regex{"^cubic[ -]cms?\\b"},         1  },
	{ std::regex{"^cubic[ -]inchs?\\b"},   16.39  },
	{ std::regex{"^cups?\\b"},             CUP    },
	{ std::regex{"^fl[ -]?oz\\b"},         CUP/8  },
	{ std::regex{"^gallons?\\b"},          CUP*16 },
	{ std::regex{"^gals?\\b"},             CUP*16 },
	{ std::regex{"^liters?\\b"},           1000   },
	{ std::regex{"^milliliters?\\b"},      1      },
	{ std::regex{"^pints?\\b"},            CUP*2  },
	{ std::regex{"^pt\\b"},                CUP*2  },
	{ std::regex{"^qt\\b"},                CUP*4  },
	{ std::regex{"^quarts?\\b"},           CUP*4  },
	{ std::regex{"^shots?\\b"},            44     },
	{ std::regex{"^tablespoons?\\b"},      CUP/16 },
	{ std::regex{"^tbsp\\b"},              CUP/16 },
	{ std::regex{"^teaspoons?\\b"},        CUP/48 },
	{ std::regex{"^tsp\\b"},               CUP/48 }
      }; // volumes

      auto ml = float{};
      for (const auto& [re, vol]: volumes) {
        if (std::regex_search(desc, re)) {
	  ml = vol;
	  break;
	}
      }
      if (ml == 0.0f)
        continue;
      ml *= value;
      std::cout << v[Idx::code] << '\t' << ml << '\t' << v[Idx::desc] << '\n';
    }
    catch (const std::exception& x) {
      std::cerr << ifname << '(' << linenum << ") " << x.what() << '\n';
    }
  }
  return EXIT_FAILURE;
} // main
