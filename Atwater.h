// Copyright 2023 Terry Golubiewski, all rights reserved.
#ifndef ATWATER_H
#define ATWATER_H
#pragma once

#include "Nutrition.h"

#include <iostream>

struct Atwater {
  static constexpr double alcohol = 6.93;
  double prot  = 4.0;
  double fat   = 9.0;
  double carb  = 4.0;
  double fiber = 0.0;
  double kcal(const Nutrition& nutr) const;
}; // Atwater

std::ostream& operator<<(std::ostream&, const Atwater&);
std::istream& operator>>(std::istream&, Atwater&);

#endif
