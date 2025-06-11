// Copyright 2023-2025 Terry Golubiewski, all rights reserved.
#ifndef NUTRITION_H
#define NUTRITION_H
#pragma once

#include <mp-units/systems/si.h>
#include <mp-units/systems/isq.h>

#include <iosfwd>
#include <compare>

namespace mp_units::si {

inline constexpr struct Calorie final : named_unit<"cal", mag_ratio<4184, 1000> * joule> {} Calorie;
inline constexpr auto Kcal = kilo<Calorie>;

} // mp_units::si

struct Nutrition {
  template<auto U> using Quantity = ::mp_units::quantity<U, float>;

  using Gram  = Quantity<::mp_units::si::gram>;
  using Litre = Quantity<::mp_units::si::litre>;
  using Kcal  = Quantity<::mp_units::si::Kcal>;

  Gram  wt;
  Litre vol;
  Kcal  energy;
  Gram  prot;
  Gram  fat;
  Gram  carb;
  Gram  fiber;
  Gram  alcohol;

  void zero() {
    wt      = Gram::zero();
    vol     = Litre::zero();
    energy  = Kcal::zero();
    prot    = Gram::zero();
    fat     = Gram::zero();
    carb    = Gram::zero();
    fiber   = Gram::zero();
    alcohol = Gram::zero();
  }

  void scaleMacros(float ratio) {
    prot    *= ratio;
    fat     *= ratio;
    carb    *= ratio;
    fiber   *= ratio;
    alcohol *= ratio;
  }

  void scale(float ratio) {
    wt     *= ratio;
    vol    *= ratio;
    energy *= ratio;
    scaleMacros(ratio);
  }

  Nutrition& operator+=(const Nutrition& rhs) {
    wt      += rhs.wt;
    vol     += rhs.vol;
    energy  += rhs.energy;
    prot    += rhs.prot;
    fat     += rhs.fat;
    carb    += rhs.carb;
    fiber   += rhs.fiber;
    alcohol += rhs.alcohol;
    return *this;
  }

  bool operator==(const Nutrition& rhs) const = default;
  auto operator<=>(const Nutrition& lhs) const = default;
}; // Nutrition

std::ostream& operator<<(std::ostream& output, const Nutrition& nutr);
std::istream& operator>>(std::istream& input, Nutrition& nutr);

#endif
