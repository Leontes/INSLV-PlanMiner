# How INSLV learns

**English** · [Español](../es/algoritmo.md)

INSLV learns fuzzy rules with the genetic algorithm of NSLV, in an incremental version that
processes the examples in episodes. This page explains what it learns, how it classifies and how
the learning works, broadly and with the names used in the code (which are in Spanish).

## What it learns

A rule says: if every attribute in its antecedent takes one of its labels, the example belongs to
its class.

```
if x1 is {Baja, Media} and x3 is {Alta} then Iris-virginica   (weight 0.97)
```

- **Fuzzy labels.** Every attribute has its partition into trapezoidal labels `[a, b, c, d]`. The
  membership of a value in a label is 1 in `[b, c]`, rises from 0 to 1 between `a` and `b`, falls
  from 1 to 0 between `c` and `d`, and is 0 outside. An unknown value fully belongs to every label.
- **Conditions with several labels.** The membership in a condition is that of its label the value
  belongs to or, if it belongs to two or more, 1: it assumes the labels form a partition, so the
  union of two overlapping labels fully covers the value. The membership in a rule is the smallest
  of its conditions. Attributes that do not appear do not count.
- **Weight.** Every rule has a weight that comes from the examples it covers:
  `(n⁺ + 1) / (n⁺ + n⁻ + 1)`, with `n⁺` and `n⁻` the sums of the memberships of the examples of
  its class and of the other classes.
- **Descriptive and discriminant.** The genetic algorithm obtains every rule in two versions with
  the same fitness: the most general one (discriminant: only the minimal conditions to separate
  the class) and the most specific one (descriptive: with every condition it can add without
  losing fitness). `Opciones::reglas` chooses which ones are returned; PlanMiner uses the
  descriptive ones.
- **Characteristic.** With noise, descriptive rules lack conditions that hold in
  almost all their examples: as each one leaves some out, it costs fitness and is not added. The
  characteristic rules (`TipoDeReglas::caracteristicas`) are the descriptive ones with every condition
  met by at least `1 - Opciones::tolerancia_al_ruido` of the examples of their class they cover. They
  are computed at the end, over all the examples: the learning is the same.
- **Comparisons between attributes.** With `Opciones::comparaciones`, a condition can compare two
  attributes with the same unit (`Variable::unidad`, other than 0; the NSLV reader takes it from
  the domain): `x1 < x2`, `x1 = x2`, `x1 <= x2`... Before learning, every pair of attributes with
  the same unit gives a new attribute, the result of comparing them (less, equal or greater), which
  is kept if its mutual information with some class is above `Opciones::umbral_de_comparaciones`
  (0.2) times the largest one of the attributes with that class. The genetic algorithm treats them
  like the others, and in the rules they become conditions with `Condicion::comparado_con`. It
  helps when the class depends on a relation between attributes, which single-attribute labels can
  only approximate with many small rules: in boxA (whether one box fits in the other), the test
  accuracy goes from 68 % to 99 % and the rules, from 77 to 15. In the other test datasets, the
  accuracy does not change beyond what varies with the seed.

## How it classifies

`clasificar` gives each example the class of the rule with the largest membership times weight.
On a tie, the rule with the larger weight wins and, if they also tie, the first one. If no rule
has a positive membership, the example gets no class. This is NSLV's inference, and it gives the
same classes as the original. A comparison holds (1) or not (0); with an unknown value, it holds.

## How it learns

### Episodes

The examples are processed in blocks of `Opciones::tamano_episodio` (5000 by default), in the
order they come. In every episode:

1. The accuracy of the existing rules on the episode's examples is measured and their weights are
   adjusted.
2. If the accuracy does not reach a threshold (100 % at first), the examples the rules already
   classify well are removed and the genetic algorithm learns new rules with the rest.
3. The new rules are analysed together with the previous ones: rules of the same class whose
   patterns fit are combined, and contradictions are reported.
4. If the new rules do not improve the accuracy on the episode, they are discarded.

With a single episode (fewer examples than its size), the genetic algorithm learns from all the
examples at once.

### The genetic algorithm

The genetic algorithm searches for one rule at a time (sequential covering): the best rule it
finds is added, and it starts again as long as it keeps adding rules.

- **Encoding.** Every individual combines a binary part (the labels each attribute accepts), an
  integer part (the class) and a real part (the relevance of each attribute and a threshold: an
  attribute whose relevance is below the threshold is not part of the rule).
- **Initial population.** The relevances start at the information measures of each attribute
  with respect to each class, computed from the examples (at random if they are all equal); the
  threshold, at 0.2.
- **Fitness.** A vector of criteria compared in order. The main one measures how much the
  classification of the episode's examples improves when the rule is added: the examples of its
  class it starts classifying well minus the examples of other classes it starts classifying
  wrong, weighted by their membership and by the rule's weight.
- **The two versions.** The most general and the most specific version with the same fitness are
  obtained for every rule, and kept in pairs.

### The random generator

It uses its own generator (`GeneradorDeAzar`), seeded with `Opciones::semilla`: the same seed and the
same data give the same rules, on any platform. It gives the same sequence as `srand` and `rand` from
glibc, because the rules depend on it, but it has no global state: it does not touch the generator
of the calling program, and `aprender` can be called from several threads at once.

To carry on with its sequence after learning, pass your own generator in `Opciones::azar`. That is
what it takes to reproduce a program that went on using `rand()` after calling the original.

## What it reports while learning

`Opciones::verbosidad` chooses how much is written to `Opciones::progreso` (or to the standard
output):

| Level | What it writes |
|-------|----------------|
| `ninguna` | nothing (the default) |
| `resumen` | a line at the start (examples, attributes, classes and episodes) and another at the end (rules and time) |
| `episodios` | also, a line per episode: its examples, the rules and the accuracy before and after, and the time |
| `detalle` | also, what happens inside every episode: information measures, the threshold, the removed examples, the added rules and the pattern analysis |
| `depuracion` | everything, including dumps of the variables, the initial population and the patterns |

```
INSLV: 135 ejemplos, 4 atributos y 3 clases; 2 episodios de hasta 70 ejemplos; reglas descriptivas
INSLV: episodio 1 de 2 (ejemplos 0 a 69): reglas 0 -> 4, acierto 0.000 -> 0.986, 0.79 s
INSLV: episodio 2 de 2 (ejemplos 70 a 134): reglas 4 -> 6, acierto 0.938 -> 1.000, 0.40 s
INSLV: 6 reglas en 1.20 s
```

The verbosity does not change the rules.

## Compared with the original

With the same data, the same seed and the default options, `aprender` gives bit by bit the same
rules as the original INSLV, and `clasificar`, the same classes as its inference (see
[Tests](tests.md)). What changes:

- It does not leak memory, does not end the program on an error (it throws exceptions) and does
  not crash when the genetic algorithm finds no rule in its first round.
- It validates the data before starting: labels with `a <= b <= c <= d` and values that belong to
  some label of their attribute.
- It does not touch the standard output unless asked to, and what it writes comes in levels.
- It also gives the discriminant rules, the weight of every rule and the episode size as an
  option, and it classifies.
- It can compare attributes. The original had something broader prepared, and switched off: new
  attributes built with functions of pairs of attributes with the same unit (comparisons,
  arithmetic and logical operations), chosen by their information and with their own place in the
  genetic algorithm. Of that, INSLV recovers the comparison, as ordinary attributes.
