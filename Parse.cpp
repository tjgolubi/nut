#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <exception>
#include <cctype>

void Parse(const std::string& line, std::vector<std::string>& row,
	   const char sep='\t', const char quote='"', const char escape='\\')
{
  auto col = row.begin();
  auto it = line.begin();
  while (it != line.end()) {
    if (col == row.end()) {
      row.emplace_back("");
      col = std::prev(row.end());
    }
    col->clear();
    while (std::isspace(*it))
      ++it;
    if (*it == quote) {
      ++it;
      while (it != line.end()) {
	if (*it == escape) {
	  if (++it == line.end())
	    break;
	}
        else if (*it == quote) {
	  if (++it == line.end() || *it != quote)
	    break;
	}
        col->push_back(*it);
	++it;
      }
      std::string_view sv{*col};
      while (sv.size() >= 2 && sv.front() == quote && sv.back() == quote) {
        sv.remove_prefix(1);
	sv.remove_suffix(1);
      }
      if (sv.size() != col->size())
        *col = std::string(sv);
    }
    else {
      while (it != line.end() && *it != sep) {
        col->push_back(*it);
	++it;
      }
    }
    while (it != line.end() && std::isspace(*it))
      ++it;
    if (it != line.end()) {
      if (*it != sep)
        throw std::runtime_error{"Parse: missing separator"};
      ++it; // skip the separator
    }
    ++col;
  }
  if (col != row.end()) {
    do {
      col->clear();
    } while (++col != row.end());
    throw std::runtime_error("Parse: invalid number of columns");
  }
} // Parse
