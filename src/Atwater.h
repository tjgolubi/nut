// Copyright 2023 Terry Golubiewski, all rights reserved.
#ifndef ATWATER_H
#define ATWATER_H
#pragma once

#include <gsl/gsl>

#include "Nutrition.h"

#include <iosfwd>
#include <string>
#include <string_view>
#include <map>

struct Atwater {
  static constexpr struct Factor final
    : ::mp_units::named_unit<"Atw", my::Kcal/mp_units::si::gram> {} Factor;
  using FactorQ = ::mp_units::quantity<Factor, float>;
  static const std::map<std::string_view, Atwater> Names;
  static constexpr FactorQ alcohol = 6.93f * Factor;
  FactorQ prot  = 4.00f * Factor;
  FactorQ fat   = 9.00f * Factor;
  FactorQ carb  = 4.00f * Factor;
  FactorQ fiber = 0.00f * Factor;
  Nutrition::Energy energy(const Nutrition& nutr) const;
  Atwater() = default;
  Atwater(float prot_, float fat_, float carb_, float fiber_=0.0f)
    : prot{prot_   * Factor}
    , fat{fat_     * Factor}
    , carb{carb_   * Factor}
    , fiber{fiber_ * Factor}
    { }
  Atwater(FactorQ prot_, FactorQ fat_, FactorQ carb_, FactorQ fiber_=0.0f * Factor)
    : prot{prot_}, fat{fat_}, carb{carb_}, fiber{fiber_} { }
  explicit Atwater(const std::string_view& sv);
  // explicit Atwater(const std::string& str);
  std::string str(std::string_view delim=" ") const;
  std::string values_str(std::string_view delim=" ") const;
  friend auto operator<=>(const Atwater&, const Atwater&) = default;
}; // Atwater

std::ostream& operator<<(std::ostream&, const Atwater&);
std::istream& operator>>(std::istream&, Atwater&);

#endif
