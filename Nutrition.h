// Copyright 2023 Terry Golubiewski, all rights reserved.
#ifndef NUTRITION_H
#define NUTRITION_H
#pragma once

#include <iosfwd>

struct Nutrition {
  float g     = 0.0;
  float ml    = 0.0;
  float kcal  = 0.0;
  float prot  = 0.0;
  float fat   = 0.0;
  float carb  = 0.0;
  float fiber = 0.0;

  void zero() {
    g     = 0.0;
    ml    = 0.0;
    kcal  = 0.0;
    prot  = 0.0;
    fat   = 0.0;
    carb  = 0.0;
    fiber = 0.0;
  }

  void scale(float ratio) {
    g     *= ratio;
    ml    *= ratio;
    kcal  *= ratio;
    prot  *= ratio;
    fat   *= ratio;
    carb  *= ratio;
    fiber *= ratio;
  }

  Nutrition& operator+=(const Nutrition& rhs) {
    g     += rhs.g;
    ml    += rhs.ml;
    kcal  += rhs.kcal;
    prot  += rhs.prot;
    fat   += rhs.fat;
    carb  += rhs.carb;
    fiber += rhs.fiber;
    return *this;
  }

  bool operator==(const Nutrition& rhs) const = default;
  auto operator<=>(const Nutrition& lhs) const = default;
}; // Nutrition

std::ostream& operator<<(std::ostream& output, const Nutrition& nutr);
std::istream& operator>>(std::istream& input, Nutrition& nutr);

#endif
