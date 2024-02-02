#include "Parse.h"
#include "Progress.h"

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

int main() {
  try {
    ConvertFile(std::cin, std::cout);
  }
  catch (const std::exception& x) {
    std::cerr << "std::exception: " << x.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
} // main
