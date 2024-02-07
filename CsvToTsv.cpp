#include "Parse.h"
#include "Progress.h"

#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <exception>
#include <iterator>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cctype>

std::ostream& Print(std::ostream& output, const std::vector<std::string>& row) {
  bool first = true;
  for (const auto& col: row) {
    if (!first)
      output << '\t';
    first = false;
    if (col.find_first_of("\t\"") != std::string::npos)
      output << std::quoted(col);
    else
      output << col;
  }
  return output << '\n';
} // Print

void ConvertFile(std::istream& input, std::ostream& output, long long filesize=0) {
  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"ConvertFile: cannot read input"};
  std::vector<std::string> row;
  int linenum = 1;
  int errCount = 0;
  ParseCsv(line, row);
  const auto numCols = row.size();
  if (numCols == 0)
    throw std::runtime_error{"ConvertFile: No column headings"};
  Print(output, row);
  auto progress = ProgressMonitor{filesize};
  while (std::getline(input, line)) {
    try {
      ++linenum;
      progress(input.tellg());
      ParseCsv(line, row);
      if (row.size() != numCols)
        throw std::runtime_error{"invalid number of columns"};
      Print(output, row);
    }
    catch (const std::exception& x) {
      std::cerr << '(' << linenum << ") " << x.what() << '\n';
      if (++errCount > 10)
        return;
    }
  }
} // ConvertFile

int main(int argc, const char* const argv[]) {
  try {
    if (argc == 1) {
      ConvertFile(std::cin, std::cout);
      return EXIT_SUCCESS;
    }
    for (int i = 1; i < argc; ++i) {
      const auto ifname = std::string{argv[i]};
      if (!ifname.ends_with(".csv")) {
        std::cerr << std::quoted(ifname) << " requires .csv extension\n";
	continue;
      }
      static const auto re = std::regex{"\\.csv$"};
      const auto ofname = std::regex_replace(ifname, re, ".tsv");
      auto input = std::ifstream{ifname, std::ios::ate};
      auto filesize = input.tellg();
      input.seekg(0);
      auto output = std::ofstream{ofname};
      std::cerr << std::quoted(ifname) << '(' << filesize << ") --> " << ofname << '\n';
      ConvertFile(input, output, filesize);
    }
  }
  catch (const std::exception& x) {
    std::cerr << "std::exception: " << x.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
} // main
