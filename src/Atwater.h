/// @file Atwater.h
/// @brief Defines the Atwater struct for nutrient-to-energy conversion.
/// @copyright Copyright 2023-2025 Terry Golubiewski, all rights reserved.
///
/// The Atwater struct defines caloric conversion factors for protein, fat,
/// carbohydrate, fiber, and alcohol, and provides interfaces to compute
/// energy content from a Nutrition profile.

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
  /// @brief Atwater metabolic specific energy.
  inline constexpr struct atwater final
    : named_unit<"Atw", kilocalorie/gram> {} atwater;
} // tjg::units

/// @brief Represents a set of nutrient metabolic specific energy factors.
struct Atwater {
  static constexpr auto atwater = tjg::units::atwater;
  using Factor = mp_units::quantity<atwater, float>;
  /// @brief Predefined Atwater factors accessible by name.
  static const std::map<std::string_view, Atwater> Names;
  static constexpr Factor alcohol = 6.93f * atwater; ///< Specific energy of alcohol.
  Factor prot  = 4.00f * atwater; ///< Specific energy of protein.
  Factor fat   = 9.00f * atwater; ///< Specific energy of fat.
  Factor carb  = 4.00f * atwater; ///< Specific energy of carbohydrates.
  Factor fiber = 0.00f * atwater; ///< Specific energy of fiber.
  /// @brief Computes total kilocalories from a Nutrition profile.
  /// @param nutr Nutrition input values.
  /// @return Total metabolic energy.
  Nutrition::Energy energy(const Nutrition& nutr) const;
  /// @brief Initializes an Atwater profile with standard default values.
  ///
  /// Sets protein = 4.0, fat = 9.0, carb = 4.0, and fiber = 0.0 kcal/g,
  /// consistent with general-purpose Atwater factors used in food labeling.
  Atwater() = default;

  /// @brief Constructs an Atwater profile with user-specified macronutrient
  /// energy values (unchecked dimensions).
  ///
  /// This constructor allows overriding any of the caloric specific energy values
  /// (protein, fat, carbohydrate, and optionally fiber).
  ///
  /// @param prot_  Specific energy of protein (kcal/g).
  /// @param fat_   Specific energy of fat (kcal/g).
  /// @param carb_  Specific energy of carbohydrates (kcal/g).
  /// @param fiber_ Specific energy of fiber (kcal/g); optional.
  Atwater(float prot_, float fat_, float carb_, float fiber_=0.0f)
    : prot{prot_   * atwater}
    , fat{fat_     * atwater}
    , carb{carb_   * atwater}
    , fiber{fiber_ * atwater}
    { }
  /// @brief Constructs an Atwater profile with user-specified macronutrient
  /// specific energy values.
  ///
  /// This constructor allows overriding any of the caloric specific energy values
  /// (protein, fat, carbohydrate, and optionally fiber).
  ///
  /// @param prot_  Specific energy of protein.
  /// @param fat_   Specific energy of fat.
  /// @param carb_  Specific energy of carbohydrates.
  /// @param fiber_ Specific energy of fiber; optional.
  Atwater(Factor prot_, Factor fat_, Factor carb_, Factor fiber_=Factor{})
    : prot{prot_}, fat{fat_}, carb{carb_}, fiber{fiber_} { }

  /// @brief Constructs an Atwater profile from a string identifier or numeric
  /// list.
  ///
  /// This constructor handles four input forms:
  /// 1. An empty string (initializes to default [4 9 4 0]).
  /// 2. A well-known name (e.g., "egg").
  /// 3. A space- or comma-delimited string of up to four float values:
  ///    protein, fat, carbohydrate, and optional fiber.
  ///
  /// Throws a std::range_error on malformed or excessive input.
  ///
  /// @throw std::range_error if the string is malformed or contains too many
  /// values.
  /// @param sv String representation of the Atwater factors or well-known name.
  explicit Atwater(const std::string_view& sv);

  /// @brief Converts the Atwater factor to a human-readable string.
  ///
  /// If the Atwater values match a known named entry,
  /// the corresponding name (e.g., "egg") is returned. Otherwise, the numeric
  /// values are returned in delimited form via values_str().
  ///
  /// @param delim Delimiter to separate values if no name match is found.
  /// @return Named label or numeric value string.
  std::string str(std::string_view delim=" ") const;

  /// @brief Converts numeric Atwater values to a formatted string.
  ///
  /// Returns the protein, fat, and carbohydrate values formatted to two
  /// decimal places.
  /// If the fiber value is non-zero, it is included as the fourth field.
  ///
  /// @param delim Delimiter to separate fields.
  /// @return String of numeric Atwater values.
  std::string values_str(std::string_view delim=" ") const;

  /// @brief Comparison operator.
  friend auto operator<=>(const Atwater&, const Atwater&) = default;
}; // Atwater

/// @brief Writes a formatted Atwater profile to an output stream.
///
/// Outputs either a symbolic name (e.g., "egg") if available,
/// or the numeric values (protein, fat, carbohydrate, [fiber]) separated by spaces.
/// The result is always enclosed in square brackets, e.g., "[egg]" or "[4.00 9.00 4.00]".
/// This relies on Atwater::str() for formatting.
///
/// @param os Output stream to write to.
/// @param atw Atwater instance to format.
/// @return Reference to the modified output stream.
std::ostream& operator<<(std::ostream& os, const Atwater& atw);

/// @brief Reads an Atwater profile from a formatted input stream.
///
/// Expects input enclosed in square brackets. The content inside the brackets
/// may be a symbolic name (e.g., "egg") or a delimited list of numeric values
/// (e.g., "4.00 9.00 4.00 [0.00]").
/// The input is passed to the Atwater(const std::string_view&) constructor for parsing.
///
/// @param is Input stream to read from.
/// @param atw Atwater instance to populate.
/// @return Reference to the modified input stream.
/// @throw std::range_error if the input is invalid or improperly formatted.
std::istream& operator>>(std::istream& is, Atwater& atw);

#endif
