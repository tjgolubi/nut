#include "../src/Parse.h"

#include <string>
#include <string_view>
#include <vector>
#include <exception>
#include <iterator>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cctype>

namespace {

std::ostream& Print(std::ostream& output, const std::vector<std::string>& row) {
  bool first = true;
  for (const auto& col: row) {
    if (!first)
      output << '\t';
    first = false;
    output << col;
  }
  return output << '\n';
} // Print

void ConvertFile(std::istream& input, std::ostream& output,
                 long long filesize=0)
{
  auto line = std::string{};
  if (!std::getline(input, line))
    throw std::runtime_error{"ConvertFile: cannot read input"};
  std::vector<std::string> row;
  int linenum = 1;
  int errCount = 0;
  ParseCsv(line, row);
  const auto numCols = row.size();
  if (numCols == 0)
    throw std::runtime_error{"ConvertFile: no column headings"};
  Print(output, row);
  auto empty = std::vector<int>(numCols);
  while (std::getline(input, line)) {
    try {
      ++linenum;
      ParseCsv(line, row);
      if (row.size() > numCols) {
        for (auto i = row.size() - 1; i != numCols; --i) {
	  if (!row[i].empty()) {
	    std::cerr << '(' << linenum << ") too many columns\n";
	    break;
	  }
	}
      }
      row.resize(numCols);
      for (int i = 0; i != numCols; ++i) {
        if (row[i].empty())
	  ++empty[i];
      }
      Print(output, row);
    }
    catch (const std::exception& x) {
      std::cerr << '(' << linenum << ") " << x.what() << '\n';
      if (++errCount > 10)
        return;
    }
  }
  for (int i=0; i != numCols; ++i) {
    if (empty[i] >= linenum-1)
      std::cerr << "********* Column " << i << " is always empty.\n";
  }
} // ConvertFile

void NewHandler() {
  std::set_new_handler(nullptr);
  std::cerr << "Out of memory!" << std::endl;
  std::terminate();
} // NewHandler

} // local

int main() {
  std::set_new_handler(NewHandler);
  try {
    ConvertFile(std::cin, std::cout);
  }
  catch (const std::exception& x) {
    std::cerr << "std::exception: " << x.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
} // main
