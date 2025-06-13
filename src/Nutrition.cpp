#include "Nutrition.h"

#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& output, const Nutrition& nutr) {
  using namespace mp_units::si;

  auto prec  = output.precision(2);
  auto flags = output.flags();
  try {
    using std::setw;
    output << std::fixed
	          << setw(8) << nutr.wt.numerical_value_in(gram)
	   << ' ' << setw(7) << nutr.vol.numerical_value_in(millilitre)
	   << ' ' << setw(7) << nutr.energy.numerical_value_in(my::Kcal)
	   << ' ' << setw(6) << nutr.prot.numerical_value_in(gram)
	   << ' ' << setw(6) << nutr.fat.numerical_value_in(gram)
	   << ' ' << setw(6) << nutr.carb.numerical_value_in(gram)
	   << ' ' << setw(6) << nutr.fiber.numerical_value_in(gram)
	   << ' ' << setw(6) << nutr.alcohol.numerical_value_in(gram);
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
  using namespace mp_units::si;

  nutr = Nutrition{};
  float wt=0, vol=0, energy=0, prot=0, fat=0, carb=0, fiber=0, alcohol=0;
  input >> wt >> vol >> energy >> prot >> fat >> carb >> fiber >> alcohol;
  nutr.wt      = wt      * gram;
  nutr.vol     = vol     * millilitre;
  nutr.energy  = energy  * my::Kcal;
  nutr.prot    = prot    * gram;
  nutr.fat     = fat     * gram;
  nutr.carb    = carb    * gram;
  nutr.fiber   = fiber   * gram;
  nutr.alcohol = alcohol * gram;
  return input;
} // >> Nutrition
