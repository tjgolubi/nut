#include "Nutrition.h"

#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& output, const Nutrition& nutr) {
  auto prec  = output.precision(2);
  auto flags = output.flags();
  try {
    using std::setw;
    output << std::fixed
	          << setw(8) << nutr.g
	   << ' ' << setw(7) << nutr.ml
	   << ' ' << setw(7) << nutr.kcal
	   << ' ' << setw(6) << nutr.prot
	   << ' ' << setw(6) << nutr.fat
	   << ' ' << setw(6) << nutr.carb
	   << ' ' << setw(6) << nutr.fiber
	   << ' ' << setw(6) << nutr.alcohol;
    output.precision(prec);
    output.flags(flags);
    return output;
  }
  catch (...) {
    output.precision(prec);
    output.setf(flags);
    throw;
  }
} // << Nutrition

std::istream& operator>>(std::istream& input, Nutrition& nutr) {
  nutr = Nutrition{};
  return input >> nutr.g >> nutr.ml >> nutr.kcal
	       >> nutr.prot >> nutr.fat >> nutr.carb >> nutr.fiber
	       >> nutr.alcohol;
} // >> Nutrition
