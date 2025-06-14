// Copyright 2023-2025 Terry Golubiewski, all rights reserved.
#ifndef NUTRITION_H
#define NUTRITION_H
#pragma once

#include <mp-units/systems/si.h>
#include <mp-units/systems/isq.h>

#include <iosfwd>
#include <compare>

struct Nutrition {
  template<auto U> using Quantity = ::mp_units::quantity<U, float>;

  using Weight = Quantity<::mp_units::si::gram>;
  using Volume = Quantity<::mp_units::si::millilitre>;
  using Energy = Quantity<::mp_units::si::kilocalorie>;

  Weight wt;
  Volume vol;
  Energy energy;
  Weight prot;
  Weight fat;
  Weight carb;
  Weight fiber;
  Weight alcohol;

  void zero() {
    wt      = wt.zero();
    vol     = vol.zero();
    energy  = energy.zero();
    prot    = prot.zero();
    fat     = fat.zero();
    carb    = carb.zero();
    fiber   = fiber.zero();
    alcohol = alcohol.zero();
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
