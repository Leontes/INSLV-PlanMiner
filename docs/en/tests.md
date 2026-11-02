# Tests

**English** · [Español](../es/pruebas.md)

Tests are grouped by CTest label:

```bash
ctest --preset debug                    # all but the slow ones
ctest --preset debug -L equivalencia    # one label
ctest --preset debug -R 'unidad.Nslv.*' # by name
ctest --preset lento                    # the slow ones, in Release
```

| Label | What it checks | Needs |
|-------|----------------|-------|
| `unidad` | the interface (`aprender`, `clasificar`, the options and the verbosity) and the NSLV format reader | GoogleTest (if it is not installed, CMake downloads it) |
| `equivalencia` | that `aprender` gives the same rules as the original and `clasificar`, the same classes: the PlanMiner cases and the fast reference rules | — |
| `lento` | the reference rules of aut, boxA and ring, which take minutes | — |
| `oraculo` | the library against the original built into the same program | `old/`, only locally |

No test preset runs the slow tests except their own (`lento`), and the sanitizer and coverage
presets do not run the oracle.

## Test data

In `tests/datos`:

| Path | Contents |
|------|----------|
| `nslv/` | five datasets in the NSLV format, each with its domain (`.dom`) and 10 cross-validation partitions (`<name>0.datos` to `<name>9.datos`) |
| `keel/aut/` | the aut partitions in the KEEL format: another partition of the same examples, to check the reader |
| `planminer/` | 26 equivalence cases: the input of INSLV in the original PlanMiner (blocksworld and depots) and the rules it returned, already interpreted |
| `referencias/` | the rules the original gives with the datasets in `nslv/`: `rapidas/` (fast) and `lentas/` (slow) |

| Dataset | Examples | Attributes | Classes | Notes |
|---------|---------:|-----------:|--------:|-------|
| irs | 150 | 4 | 3 | |
| wne | 178 | 13 | 3 | |
| aut | 205 | 25 | 6 | unknown values, nominal attributes with up to 22 labels |
| boxA | 1000 | 6 | 3 | |
| ring | 7400 | 20 | 2 | with 9 partitions it takes more than one episode (5000 examples) |

## Reference rules

Each file in `tests/datos/referencias` holds the rules the original gave in an experiment, named
by the file, and the classes its inference gives with them to the examples of the test partition:

```
<dataset>-test<k>-<descriptivas|discriminantes>[-episodio<n>].reglas
```

The rules are learned from every partition of the dataset but `k`, with the default seed and
episodes of `n` examples (5000 if not given). The test learns the same and compares the rules bit
by bit (the class, the covered examples, the weight and every condition with its labels) and what
`clasificar` gives with them to partition `k`.

| Group | Experiments |
|-------|-------------|
| `rapidas/` | irs with each of its 10 partitions as test; wne with partition 0; irs and wne with episodes of 20 and 50 examples (the incremental part). Descriptive and discriminant |
| `lentas/` | aut, boxA and ring with partition 0, descriptive and discriminant |

They are generated with the original, so they can only be regenerated locally, with `old/`:

```bash
cmake --build --preset debug --target inslv_generar_referencias
ls tests/datos/referencias/*/*.reglas \
  | xargs -P "$(nproc)" -n 1 build/debug/tests/oraculo/inslv_generar_referencias
```

To add an experiment, pass it the name of the new file. The original uses the global random
generator and redirects the standard output: each file is generated in its own process.

## The oracle

`old/` is the original implementation, which is only kept locally. If it is there, CMake builds it
as `inslv_legacy`, with its namespace changed by a macro (`-Dinslv=inslv_legacy`) so that it can be
linked into the same program as the library, and registers the tests labelled `oraculo`: for every
PlanMiner case, the descriptive and the discriminant rules have to be bit by bit those of the
original, and `clasificar` has to give the examples the same classes as its inference
(`ruleset::InferenceC`). It is turned off with `-DINSLV_ORACULO=OFF`.

## Sanitizers and coverage

```bash
cmake --workflow --preset asan       # AddressSanitizer and UBSan
cmake --workflow --preset cobertura  # coverage with gcov (GCC)
cmake --build --preset cobertura --target informe_cobertura
```

- **Sanitizers:** no suppressions: any memory leak or undefined behaviour makes the tests fail.
  The three leaks of the original (about 800 bytes per call) are fixed.
- **Coverage:** the library and its sub-libraries are instrumented, not the tests. The report is
  written to `build/cobertura/cobertura/` (`index.html` and `resumen.txt`); it needs gcovr.

## Continuous integration

It builds and runs the tests with GCC and Clang (warnings as errors), the slow tests, the
sanitizers, clang-tidy and the formatting check (clang-format 18). It does not have `old/`: the
equivalence with the original is checked with the stored cases and reference rules.
