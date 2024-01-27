// Copyright 2023 Terry Golubiewski, all rights reserved.
#ifndef ATWATER_H
#define ATWATER_H
#pragma once

#include <gsl/gsl>

#include "Nutrition.h"

#include <iosfwd>
#include <string>
#include <map>

struct Atwater {
  static const std::map<std::string, Atwater> Names;
  static constexpr float alcohol = 6.93f;
  float prot  = 4.0f;
  float fat   = 9.0f;
  float carb  = 4.0f;
  float fiber = 0.0f;
  float kcal(const Nutrition& nutr) const;
  Atwater() = default;
  Atwater(float prot_, float fat_, float carb_, float fiber_=0.0f)
    : prot{prot_}, fat{fat_}, carb{carb_}, fiber{fiber_} { }
  explicit Atwater(const std::string& str);
  explicit Atwater(const std::string_view& sv) : Atwater(std::string{sv}) { }
  std::string str(gsl::not_null<const char*> delim=" ") const;
  std::string values_str(gsl::not_null<const char*> delim=" ") const;
  friend auto operator<=>(const Atwater&, const Atwater&) = default;
}; // Atwater

std::ostream& operator<<(std::ostream&, const Atwater&);
std::istream& operator>>(std::istream&, Atwater&);

#endif
