# INSLV

**English** · [Español](README.es.md)

Fuzzy rule learning with the genetic algorithm of NSLV, in its incremental version: the examples
are processed in episodes. INSLV learns the rules that
[PlanMiner](https://github.com/Leontes/PlanMiner) turns into the preconditions and effects of
planning actions.

This repository ports INSLV to modern C++ (C++20). **Work in progress:** for now the
implementation is the original one, copied unchanged as the starting point, behind the same
interface PlanMiner uses (`inslv::learn`). It is modernized step by step, and every step has to
give exactly the same rules as the original.

## Goals

- **The same rules as the original**, checked against it at every step.
- **Descriptive and discriminant rules.** INSLV was tuned for descriptive rules, the ones
  PlanMiner needs; the new version will also give discriminant rules (only the minimal
  conditions in the antecedent).
- **Classification**, not only rule learning.
- **A reader for the NSLV format** (`.dom` domains and `.datos` examples).
- A library without global state, memory leaks or calls that end the program.

## Usage

```cpp
#include "inslv/inslv.hpp"

inslv::Dataset data;
data.attributes = {{"a", {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}}};
data.target = {"Class", {{"Negative", 0, 0, 0, 0}, {"Positive", 1, 1, 1, 1}}};
data.values = {{0, 1, 1, 0}};  // values[attribute][example]; NaN: unknown
data.classes = {0, 1, 1, 0};   // index of a target label

for (const inslv::Rule& rule : inslv::learn(data)) {
  // rule.antecedent: attributes and the labels they accept; rule.consequent: the class;
  // rule.covered: positive examples covered
}
```

```cmake
add_subdirectory(inslv)  # or FetchContent
target_link_libraries(my_program PRIVATE inslv::inslv)
```

The interface will change when the port reaches it: names in Spanish, discriminant rules and
classification.

## Structure

| Path | Contents |
|------|----------|
| `include/inslv/` | the public header (`inslv.hpp`) and, in a folder per sub-library, the internal headers |
| `src/` | the interface (`inslv.cpp`) and the sub-libraries, each in its folder: |
| `src/datos/` | the examples |
| `src/difuso/` | fuzzy logic: labels, domains and variables |
| `src/genetico/` | the genetic algorithm: the genetic code, the populations and their operators |
| `src/reglas/` | the rule set and its inference |
| `src/aprendizaje/` | NSLV's incremental learning |
| `tests/` | unit, equivalence and oracle tests; `apoyo/`: what they share |
| `tests/datos/nslv/` | five datasets in the NSLV format, each with its domain and 10 cross-validation partitions: aut, boxA, irs, ring and wne |
| `tests/datos/keel/` | the aut partitions in the KEEL format, to test the reader |
| `tests/datos/planminer/` | 26 equivalence cases: the input of INSLV in the original PlanMiner and the rules it returned |
| `cmake/` | CMake modules: warnings, sanitizers, coverage, sub-libraries, dependencies and the oracle |

`build/` (builds), `old/` (the original code, the tests' oracle) and `privado/` (private
documents) are not in git.

## Development

```bash
cmake --workflow --preset debug                     # configure, build and test
cmake --workflow --preset asan                      # with AddressSanitizer and UBSan
cmake --workflow --preset cobertura                 # coverage (GCC), then the report:
cmake --build --preset cobertura --target informe_cobertura  # build/cobertura/cobertura/
cmake --preset tidy && cmake --build --preset tidy  # clang-tidy on the new code
git ls-files -- '*.cpp' '*.hpp' '*.h' | xargs clang-format --dry-run --Werror
```

- **Tests:** by CTest label (`ctest --preset debug -L unidad`): `unidad` (the interface),
  `equivalencia` (the rules against the ones the original gave, stored in `tests/datos`) and
  `oraculo` (only locally, with `old/`: the library against the original built into the same
  program).
- **Code inherited from the original:** for now every sub-library holds the original code
  (`HEREDADA` in its `CMakeLists.txt`). It is built without the new code's warnings, formatting
  or clang-tidy until it is modernized. Everything else is built with strict warnings, and the
  continuous integration treats them as errors.
- **Test dependencies:** GoogleTest 1.14 or later (if it is not installed, CMake downloads it);
  for coverage, gcovr.
- **Continuous integration:** builds and tests with GCC and Clang, with the sanitizers and with
  clang-tidy, and checks the formatting (clang-format 18).

## License

The code and the documentation are distributed under the
[GNU General Public License version 3](LICENSE) (`GPL-3.0-only`). INSLV is based on NSLV, by
Raúl Pérez (© 2002–2003); the port is © 2026 José Segura. The test datasets keep the terms of
their originals. See [LICENSES.md](LICENSES.md).
