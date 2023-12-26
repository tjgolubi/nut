// Copyright 2023 Terry Golubiewski, all rights reserved.

#include <gsl/gsl>

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#include "Nutrition.h"

int main() {
  using std::cout;
  try {
    std::string name;
    Nutrition nutr;

    std::ifstream input{"ingred.dat", std::ios::binary};

    std::vector<std::pair<std::string, Nutrition>> v;

    while (std::getline(input, name, '\0')) {
      input.read(reinterpret_cast<char*>(&nutr), sizeof(nutr));
      v.emplace_back(name, nutr);
    }

    std::string::size_type w = 0;
    for (auto const& [x, y]: v)
      w = std::max(w, x.size());

    for (auto const& [x, y]: v) {
      std::cout << std::left << std::setw(w) << x << std::right
	  << ' ' << y << '\n';
    }

    return EXIT_SUCCESS;
  }
  catch (const std::ios::failure& fail) {
    cout << "ios::failure: " << fail.what()
              << "\n    error code = " << fail.code().message() << std::endl;

  }
  catch (const std::exception& x) {
    cout << "standard exception: " << x.what() << std::endl;
  }

  return EXIT_FAILURE;
} // main
