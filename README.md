# INSLV

**English** · [Español](README.es.md)

Fuzzy rule learning with the genetic algorithm of NSLV, in its incremental version: the examples
are processed in episodes. INSLV learns the rules that
[PlanMiner](https://github.com/Leontes/PlanMiner) turns into the preconditions and effects of
planning actions.

This repository ports INSLV to modern C++ (C++20). **Work in progress:** the implementation is
still the original one, without its dead code and with its failures fixed, behind a new interface
in Spanish (`inslv::aprender` and `inslv::clasificar`). It is modernized step by step, and every
step has to give exactly the same rules as the original.

## What it adds to the original

- **The same rules**, checked against the original at every step, and the same classes as its
  inference.
- **Descriptive and discriminant rules.** INSLV was tuned for descriptive rules, the ones
  PlanMiner needs; it now also gives the discriminant ones (only the minimal conditions in the
  antecedent).
- **Classification** (`clasificar`), not only rule learning.
- **A reader for the NSLV format** (`.dom` domains and `.datos` examples).
- **No memory leaks, no calls that end the program, no use of the standard output** unless asked
  for, with a levelled verbosity. What remains is the global random generator (`rand`).

## Usage

```cpp
#include "inslv/inslv.hpp"

inslv::Datos datos;
datos.atributos = {{"a", {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}}};
datos.clase = {"Clase", {{"Negativa", 0, 0, 0, 0}, {"Positiva", 1, 1, 1, 1}}};
datos.valores = {{0, 1, 1, 0}};  // valores[attribute][example]; NaN: unknown
datos.clases = {0, 1, 1, 0};     // index of a label of the class

std::vector<inslv::Regla> reglas = inslv::aprender(datos);
for (const inslv::Regla& regla : reglas) {
  // regla.antecedente: attributes and the labels they accept; regla.consecuente: the class;
  // regla.cubiertos: positive examples covered; regla.peso: its weight when classifying
}
// One class per example, or none if no rule fires.
std::vector<std::optional<std::size_t>> clases = inslv::clasificar(reglas, datos);
```

The options (`inslv::Opciones`) choose the descriptive rules (the default, the ones PlanMiner
uses) or the discriminant ones, with only the minimal conditions; the episode size; and how much
the learning reports (`verbosidad`, from `ninguna` to `depuracion`) and where
(`progreso`, an `std::ostream*`; the standard output by default). Datasets in the NSLV format are
read with `inslv/formatos/nslv.hpp`:

```cpp
#include "inslv/formatos/nslv.hpp"

inslv::Datos datos = inslv::nslv::leer("irs.dom", {"irs1.datos", "irs2.datos"});
inslv::Opciones opciones;
opciones.reglas = inslv::TipoDeReglas::discriminantes;
opciones.tamano_episodio = 50;  // examples per episode (5000 by default)
opciones.verbosidad = inslv::Verbosidad::episodios;  // a line per episode
std::vector<inslv::Regla> reglas = inslv::aprender(datos, opciones);
```

```cmake
add_subdirectory(inslv)  # or FetchContent
target_link_libraries(my_program PRIVATE inslv::inslv)
```

The interface is in Spanish, like the rest of the code. See
[How INSLV learns](docs/en/algorithm.md).

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
| `src/comun/` | what the sub-libraries share: the progress stream |
| `src/formatos/` | the NSLV format reader, apart from the engine |
| `tests/` | unit, equivalence and oracle tests; `apoyo/`: what they share |
| `tests/datos/nslv/` | five datasets in the NSLV format, each with its domain and 10 cross-validation partitions: aut, boxA, irs, ring and wne |
| `tests/datos/keel/` | the aut partitions in the KEEL format, to test the reader |
| `tests/datos/planminer/` | 26 equivalence cases: the input of INSLV in the original PlanMiner and the rules it returned |
| `tests/datos/referencias/` | the rules the original gives with the NSLV datasets |
| `docs/es/`, `docs/en/` | documentation, in Spanish and English |
| `cmake/` | CMake modules: warnings, sanitizers, coverage, sub-libraries, dependencies and the oracle |

`build/` (builds), `old/` (the original code, the tests' oracle) and `privado/` (private
documents) are not in git.

## Documentation

- [How INSLV learns](docs/en/algorithm.md): what it learns, how it classifies, episodes,
  the genetic algorithm and the verbosity levels.
- [Tests](docs/en/tests.md): labels, test data, reference rules, the oracle, sanitizers and
  coverage.
- [Licenses](LICENSES.md): the code's, NSLV's and the test data's.

## Development

```bash
cmake --workflow --preset debug                     # configure, build and test
cmake --workflow --preset asan                      # with AddressSanitizer and UBSan
cmake --workflow --preset cobertura                 # coverage (GCC), then the report:
cmake --build --preset cobertura --target informe_cobertura  # build/cobertura/cobertura/
cmake --preset tidy && cmake --build --preset tidy  # clang-tidy on the new code
git ls-files -- '*.cpp' '*.hpp' '*.h' | xargs clang-format --dry-run --Werror
```

- **Tests:** by CTest label (`ctest --preset debug -L unidad`): `unidad` (the interface and the
  reader), `equivalencia` (the rules against the ones the original gave, stored in
  `tests/datos`), `lento` (the same with the large datasets: `ctest --preset lento`) and
  `oraculo` (only locally, with `old/`: the library against the original built into the same
  program). See [Tests](docs/en/tests.md).
- **Code inherited from the original:** `aprendizaje`, the only sub-library not yet modernized,
  holds the original code (`HEREDADA` in its `CMakeLists.txt`). It is built without the new
  code's warnings, formatting or clang-tidy until it is modernized. Everything else is built with strict warnings, and the
  continuous integration treats them as errors.
- **Test dependencies:** GoogleTest 1.14 or later (if it is not installed, CMake downloads it);
  for coverage, gcovr.
- **Continuous integration:** builds and tests with GCC and Clang, runs the slow tests, the
  sanitizers and clang-tidy, and checks the formatting (clang-format 18).

## License

The code and the documentation are distributed under the
[GNU General Public License version 3](LICENSE) (`GPL-3.0-only`). INSLV is based on NSLV, by
Raúl Pérez (© 2002–2003); the port is © 2026 José Segura. The test datasets keep the terms of
their originals. See [LICENSES.md](LICENSES.md).
