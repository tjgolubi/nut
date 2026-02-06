Title: Ingredient File Syntax

# Comments

C++-style comments are supported, i.e. portions of the line after and including
"//" are ignored.

Lines that begin with '#' are comments if they are preprocessor directives
[(see below](Preprocessor Directives).  They are typically used in the style
of Markdown headings to organize ingredients but are ignored.

Leading- and trailing- whitespace is ignored.

Blank lines are ignored.

# Preprocessor Directives

Control macro definitions, file inclusion, and conditional parsing.

``` C++
#include "file.txt"
#define VAR value
#undef VAR
#ifdef VAR
#ifndef VAR
#if 0
#else
#endif
#echo message
```

Notice that "#if {expression}" is not supported.  "#if 0" is used to comment-out
a block of lines.

``` C++
#if 0
ignore
these
lines
#endif
```

# Atwater Factors

Defines the energy-per-gram values (kcal/g) for macronutrients.

```
[4.0 9.0 3.5 2.0]
[3.9 8.7 2.6]
[bean]
[general]
```
- Order: protein, fat, carbohydrate, optional fiber (defaults to 0).
- Active values are applied when computing kcal from macronutrients.
- Initially [4 9 4] ([general]) is used.
- The Atwater scale factor for alcohol is fixed at 6.93 kcal/g.

The Atwater factor applies to all nutrient records
[(see below)](Nutrient Records) until the next Atwater factor.

See [wikipedia](https://en.wikipedia.org/wiki/Atwater_system) for
information about the Atwater System.

# Variable Definitions

Declares variables that can be expanded later using `$varname`.

```
: chicken of the sea               // assigns $$
: fish = albacore
: in = water
: packet = packet of $fish in $in $$
#echo $packet
: in = oil
#echo $packet
: fish = yellow fin
#echo $oil_packet
: fish =                           // deletes $fish
#echo tuna $fish
:                                  // deletes all variables (including $$)
#echo "$water_packet" and "$fish"
```
... results in ...

```
ingred.txt(5) packet of albacore in water chicken of the sea
ingred.txt(6) packet of albacore in oil chicken of the sea
ingred.txt(8) packet of albacore in oil chicken of the sea
ingred.txt(10) tuna $fish
ingred.txt(12) "$water_packet" and "$fish"
```

Variables are not expanded prior to assigning to "$$".
Variables are expanded prior to assigning to other variables.

**TODO** Make variable expansion recursive, so that the line (8) reads
"yellow fin" instead of "albacore".

# Nutrition Records

Nutrition records define the nutrient profile of an ingredient. There are three types:

- [basic nutrition record](Basic Nutrition Record)
- [derived nutrition record](Derived Nutrition Record)
- [clone nutrition record](Clone Nutrition Record)

All nutrient records may begin with an optional asterisk (`*`) which allows
recipes to specify a quantity of this ingredient in terms of portions or items,
i.e. "each" makes sense.

All nutrient records end with a `description` that must be unique for each
ingredient.  The `description` must be lowercase.  If the `description` contains
any of "!$()\*+:;<=>?@[]^{|}~", text after and including that character will be
ignored.

## Basic Nutrition Record

Specifies full nutrient profile directly:

```
[*] weight volume kcal[?] protein fat carbs fiber/alcohol description
[*] weight volume   =     protein fat carbs fiber/alcohol description
```

- `weight` = portion weight in grams       (0 means unknown or not applicable)
- `volume` = portion volume in milliliters (0 means unknown or not applicable)
- `kcal` (or `=`) = portion energy in kilocalories
- `protein` `fat` `carbs` `fiber/alcohol` = macronutrients in grams

A negative value of `fiber/alcohol` designates an alcohol (with no fiber).

The `kcal` value is checked for consistency with the specified macronutrients
using the prevailing Atwater factors, unless '?' is used to disable checking.
If `kcal` is `=`, then the portion energy is calculated from the macronutrients
and Atwater factors.

If both the `weight` and `volume` fields are zero, the record is
interpreted as describing an abstract or symbolic portion. This is
stronger than prepending an asterisk (*) to the record line. While '*'
indicates that the ingredient *may* be used in recipes by portion (e.g.,
`each`), specifying both `weight` and `volume` as zero means the ingredient
*must* be used this way. No weight or volume may be given in recipes
that reference such an ingredient. In such cases, the only way to
reference the ingredient in a recipe is by quantity of portions (e.g.,
`2 each watermelon`, `3.5 each cotton candy`), because specifying a mass
or volume (e.g., `5 g of egg`) would be invalid.

## Derived Nutrition Record

Derived from a previously defined ingredient. Macronutrients are copied or scaled.

```
[*] weight volume kcal prior description
[*] weight volume kcal this  description
[*] weight volume  =   prior description
[*] weight volume  =   this  description
```

- `prior` description of prior ingredient (must be quoted if it contains spaces)
- `this`  refers to the current base record (must not be quoted)
- `kcal` can be numeric (scale macros) or `=` (copy macros)
- If any of `g`, `ml`, or `kcal` is zero, values are taken from the base record
- `name == "replace"` updates the prior record

This override behavior for `g` and `ml` is intentional: it allows the derived record to represent a new portion size that shares the same nutritional density as the base record. For example, a 50 g serving of “cooked rice” can be used as the base for a 100 g portion of “seasoned rice,” with calories and macronutrients scaled accordingly.

## Clone Nutrition Record

Creates a new name pointing to an existing record, with no data changes.

```
[*] = "prior" name // (1)
[*] = this name    // (2)
```

Defines a new ingredient record, named `name` with the same values as a prior
record.

(1) The prior record is named "prior".
(2) The prior record is the current base record.

# Clone Generation

Pattern-based clone generation

```
/pattern/replacement/
```
For each previous ingredient record whose description contains `pattern`,
a new clone is created where its description is created by replacing `pattern`
with `replacement`.
