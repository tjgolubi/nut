#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <exception>
#include <cctype>

#include <iostream>
#include <iomanip>

namespace {

void RemoveTabs(std::string& str) {
  if (!str.contains('\t'))
    return;
  static const auto re = std::regex{"\\t\\s*"};
  str = std::regex_replace(str, re, " ");
} // RemoveTabs

void RemoveExcessQuotes(std::string& str) {
  if (!str.contains('"'))
    return;
  static const auto re = std::regex{"\"\"+"};
  str = std::regex_replace(str, re, "\"");
} // RemoveExcessQuotes

bool TrimSpaces(std::string& str) noexcept {
  if (str.empty() || !std::isspace(str.front()) && !std::isspace(str.back()))
    return false;
  while (!str.empty() && std::isspace(str.back()))
    str.pop_back();
  auto iter = str.begin();
  while (iter != str.end() && std::isspace(*iter))
    ++iter;
  str.erase(str.begin(), iter);
  return true;
} // TrimSpaces

bool TrimQuotes(std::string& str, char quote='"') noexcept {
  auto sv = std::string_view{str};
  while (sv.size() >= 2 && sv.front() == quote && sv.back() == quote) {
    sv.remove_prefix(1);
    sv.remove_suffix(1);
  }
  if (sv.size() == str.size())
    return false;
  str = std::string{sv};
  return true;
} // TrimQuotes

class Spc {
  int _n;
public:
  explicit Spc(int n_) : _n(n_) { }
  friend std::ostream& operator<<(std::ostream& os, const Spc& s) {
    auto n = s._n;
    while (n-- != 0)
      os << ' ';
    return os;
  }
}; // Spc

} // local

void Parse(const std::string& line, std::vector<std::string>& row,
           const char sep, const char quote, const char escape)
{
  if (line.empty()) {
    row.clear();
    return;
  }
  auto col = row.begin();
  auto it = line.begin();
  while (it != line.end()) {
    if (col == row.end()) {
      row.emplace_back("");
      col = std::prev(row.end());
    }
    col->clear();
    if (*it == quote) {
      ++it;
      while (it != line.end()) {
        if (*it == escape) {
          if (++it == line.end())
            break;
        }
        else if (*it == quote) {
          break;
        }
        col->push_back(*it);
        ++it;
      }
      if (it == line.end() || *it != quote)
        throw std::runtime_error{"Parse: missing quote"};
      if (++it != line.end() && *it != sep)
        throw std::runtime_error{"Parse: missing separator"};
    }
    else {
      while (it != line.end() && *it != sep) {
        col->push_back(*it);
        ++it;
      }
    }
    RemoveTabs(*col);
    {
      auto changed = true;
      while (changed) {
        changed = TrimSpaces(*col);
        changed = TrimQuotes(*col) || changed;
      }
    }
    RemoveExcessQuotes(*col);
    ++col;
    if (it == line.end() || *it != sep)
      break;
    ++it;
  }
  if (it != line.end())
    throw std::runtime_error{"Parse: missing separator"};
  row.erase(col, row.end());
} // Parse
