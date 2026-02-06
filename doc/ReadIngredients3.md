## Function Overview: ReadIngredients()

`ReadIngredients()` is the core parser function in `digest.cpp`. It interprets and compiles a custom domain-specific language (DSL) defined in `ingred.txt` and its included files into a binary database of named `Nutrition` records.

This function reads annotated text files describing ingredient nutrition data and directives. It applies preprocessing logic, expands macros, resolves conditional blocks, parses nutrient records, validates kcal estimates, and emits a structured nutrition map for use in downstream tools.

### Function Signature

```cpp
void ReadIngredients(const std::string& fname,
                     NutritionMap& nuts,
                     VarMap& defs);
```

- `fname`: path to the input ingredient file (e.g., `ingred.txt`)
- `nuts`: output map from ingredient names to `Nutrition` values
- `defs`: global macro definitions carried across file inclusions

### Key Responsibilities

- Opens and reads the specified file line-by-line
- Applies preprocessing:
  - `#include` to support file modularity
  - `#define` / `#undef` to manage macros
  - Conditional blocks: `#ifdef`, `#ifndef`, `#else`, `#endif`
- Interprets and substitutes:
  - `$var`, `$$` and `:var = value` constructs
- Parses:
  - Atwater factor declarations
  - Nutrition records: direct, derived, or scaled
  - Equivalence assignments and validation ranges
- Transforms:
  - Ingredient name synonyms
  - Pattern-based renaming (`/pattern/replacement/`)
- Emits records into the `nuts` map for final binary serialization

---

### Parsed File Tree (as of scan)

```text
ingred.txt
├── defs.txt
├── chicken.txt
├── turkey.txt
├── groundbeef.txt
├── steak.txt
├── branded/ChickenOfTheSea.txt
├── branded/BumbleBee.txt
└── branded/StarKist.txt
```

Total: 9 files, ~3700 lines analyzed.

---

### DSL Line Types in `ingred.txt`

#### 1. Preprocessor Directives

Control macro definitions and file inclusion.

```
#include "file.txt"
#define VAR value
#undef VAR
#ifdef VAR / #ifndef VAR / #else / #endif
#echo message
```

#### 2. Atwater Factors

Defines kcal/g for nutrients (prot, fat, carb, optional fiber):

```
[4.0 9.0 3.5 2.0]
```

#### 3. Variable Definitions (Colon Prefix)

Declares a local variable usable as `$varname`:

```
:serving_size = 100
```

#### 4. Substitution Rules

Performs pattern-based renaming of ingredient names:

```
/pattern/replacement/
```

#### 6. Nutrition Records

The `ingred.txt` DSL supports multiple syntactic forms for Nutrition Records. These describe how a portion of an ingredient is defined either directly or by reference to a prior record. The syntax supports:

##### 6.1 Basic Nutrition Record

Defines a new ingredient directly by specifying weight, volume, calories, and macronutrient values (protein, fat, carbs, fiber or alcohol).

Syntax:
```
[*] g ml kcal[?] prot fat carb fiber/alcohol name
[*] g ml = prot fat carb fiber/alcohol name
```
- `*` is optional and allows portion-based usage
- `kcal` may be `=` to auto-compute from macronutrients
- `?` after `kcal` relaxes kcal validation
- Negative fiber indicates alcohol

##### 6.2 Derived Nutrition Record

Creates a new record derived from an existing one, either scaled by energy or directly copied.

Syntax:
```
[*] g ml kcal "prior" name
[*] g ml kcal this name
[*] g ml = "prior" name
[*] g ml = this name
```
- `prior` may be quoted or replaced by the keyword `this`
- `kcal =` copies values directly
- Numeric `kcal` scales values
- `g` and `ml` may override base values or be inherited
- Special name `replace` updates the base record instead of creating a new one

##### 6.3 Clone Nutrition Record

Assigns a new name to an existing ingredient without modifying any nutrient data.

Syntax:
```
[*] = "prior" name
[*] = this name
```
- No numeric fields permitted
- Only the name changes

For complete validation logic, field resolution, and edge cases like symbolic portions and synonym application, see the full DSL reference:

📄 [DSL_Line_Types.md](DSL_Line_Types.md)

*(Further sections will be added as we walk through each major part of the function.)*
