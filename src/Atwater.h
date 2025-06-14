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

namespace tjg::units {
  using namespace mp_units;
  using namespace mp_units::si;
  inline constexpr struct atwater final
    : named_unit<"Atw", kilocalorie/gram> {} atwater;
} // tjg::units

struct Atwater {
  using Factor = mp_units::quantity<tjg::units::atwater, float>;
  static const std::map<std::string_view, Atwater> Names;
  static constexpr Factor alcohol = 6.93f * tjg::units::atwater;
  Factor prot  = 4.00f * tjg::units::atwater;
  Factor fat   = 9.00f * tjg::units::atwater;
  Factor carb  = 4.00f * tjg::units::atwater;
  Factor fiber = 0.00f * tjg::units::atwater;
  Nutrition::Energy energy(const Nutrition& nutr) const;
  Atwater() = default;
  Atwater(float prot_, float fat_, float carb_, float fiber_=0.0f)
    : prot{prot_   * tjg::units::atwater}
    , fat{fat_     * tjg::units::atwater}
    , carb{carb_   * tjg::units::atwater}
    , fiber{fiber_ * tjg::units::atwater}
    { }
  Atwater(Factor prot_, Factor fat_, Factor carb_, Factor fiber_=Factor{})
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
