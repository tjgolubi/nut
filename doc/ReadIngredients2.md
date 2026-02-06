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

...

*(Further sections will be added as we walk through each major part of the function.)*
