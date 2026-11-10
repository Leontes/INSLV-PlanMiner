# Licenses

**English** · [Español](LICENCIAS.md)

The code of this repository and the documentation are distributed under the
[GNU General Public License version 3](LICENSE), that version only (`GPL-3.0-only`).

## NSLV

INSLV is based on NSLV, by Raúl Pérez (© 2002–2003), under the GNU GPL version 2 or later, as
stated in the headers of its sources. That option to use a later version is what allows it to
be distributed under version 3.

## Test data

- `tests/datos/nslv/`: irs, wne and aut are Iris, Wine and Automobile, from the UCI Machine
  Learning Repository. ring is Ringnorm, Leo Breiman's artificial problem, in the KEEL version
  (taken from DELVE). They follow the terms of their originals. boxA is by Raúl Pérez, who built
  it while developing NSLV to see whether an algorithm detects complex relations between
  attributes: it is not a public benchmark.
- `tests/datos/keel/`: the Automobile partitions in the KEEL format. They follow the terms of
  their original.
- `tests/datos/planminer/`: the cases come from the blocksworld and depots test traces of the
  original PlanMiner. They follow the terms of their originals.
