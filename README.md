# INSLV

**English** · [Español](README.es.md)

Fuzzy rule learning with the genetic algorithm of NSLV, in its incremental version: the examples
are processed in episodes. INSLV learns the rules that
[PlanMiner](https://github.com/Leontes/PlanMiner) turns into the preconditions and effects of
planning actions.

This repository ports INSLV to modern C++ (C++20). **Work in progress:** there is no code here
yet. Until the port is ready, the version in use is the one in PlanMiner (`vendor/inslv`).

## Goals

- **The same rules as the original**, checked against it at every step.
- **Descriptive and discriminant rules.** INSLV was tuned for descriptive rules, the ones
  PlanMiner needs; the new version will also give discriminant rules (only the minimal
  conditions in the antecedent).
- **Classification**, not only rule learning.
- **A reader for the NSLV format** (`.dom` domains and `.datos` examples).
- A library without global state, memory leaks or calls that end the program.

## Structure

| Path | Contents |
|------|----------|
| `tests/datos/nslv/` | five datasets in the NSLV format, each with its domain and 10 cross-validation partitions: aut, boxA, irs, ring and wne |
| `tests/datos/planminer/` | 26 equivalence cases: the input of INSLV in the original PlanMiner and the rules it returned |

`build/` (builds), `old/` (the original code, the tests' oracle) and `privado/` (private
documents) are not in git.

## License

The code and the documentation are distributed under the
[GNU General Public License version 3](LICENSE). INSLV is based on NSLV, by Raúl Pérez
(© 2002–2003); the port is © 2026 José Segura. The test datasets keep the terms of their
originals. See [LICENSES.md](LICENSES.md).
