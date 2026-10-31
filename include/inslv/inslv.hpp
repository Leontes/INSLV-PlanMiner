// SPDX-License-Identifier: GPL-3.0-only
// INSLV: aprendizaje de reglas difusas con el algoritmo genético de NSLV, en su versión
// incremental (los ejemplos se procesan por episodios).
//
// Esta es la única cabecera pública: las clases internas (VectorVar, example_set, ruleset...)
// no forman parte de la interfaz.
#pragma once

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace inslv {

/// Etiqueta difusa trapezoidal: [a, b, c, d], con a <= b <= c <= d. El valor central de la
/// etiqueta es (b + c) / 2. Las etiquetas no se extienden hasta el infinito por los extremos.
struct FuzzySet {
  std::string label;
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  double d = 0.0;
};

/// Una variable con su partición difusa.
struct Variable {
  std::string name;
  std::vector<FuzzySet> labels;
};

/// El conjunto de ejemplos, por columnas.
///
/// - values[i][e]: valor del atributo i en el ejemplo e. NaN significa desconocido: se adapta
///   por completo a todas las etiquetas.
/// - classes[e]: la clase del ejemplo e, como índice de una etiqueta de target.
struct Dataset {
  std::vector<Variable> attributes;
  Variable target;
  std::vector<std::vector<double>> values;
  std::vector<std::size_t> classes;
};

/// Una condición del antecedente: el atributo toma alguna de las etiquetas (índices en
/// Dataset::attributes y Variable::labels).
struct Condition {
  std::size_t attribute = 0;
  std::vector<std::size_t> labels;
};

/// Una regla: si se cumplen todas las condiciones, el ejemplo es de la clase consequent.
struct Rule {
  std::vector<Condition> antecedent;
  std::size_t consequent = 0;
  /// Ejemplos positivos que cubre la regla (n+ en NSLV).
  double covered = 0.0;
  /// Peso de la regla al clasificar: gana la regla con mayor adaptación del ejemplo por su peso.
  double weight = 0.0;
};

/// Qué versión de cada regla devuelve learn. El genético obtiene las dos a la vez, con la misma
/// bondad.
enum class RuleType {
  /// La más específica. Son las que usa PlanMiner.
  descriptive,
  /// La más general: solo las condiciones mínimas del antecedente.
  discriminant,
};

struct Options {
  /// Semilla del generador de números aleatorios. Con la misma semilla y los mismos datos el
  /// resultado es el mismo.
  unsigned seed = 0;
  /// Si es true y no hay progress, el progreso del aprendizaje sale por la salida estándar.
  bool verbose = false;
  /// Si no es nulo, el progreso del aprendizaje (los episodios, el genético...) se escribe aquí.
  std::ostream* progress = nullptr;
  /// Si no está vacío, el informe del aprendizaje (medidas y reglas) se escribe en este fichero.
  std::filesystem::path report;
  /// Qué versión de las reglas se devuelve.
  RuleType rules = RuleType::descriptive;
  /// Ejemplos por episodio del aprendizaje incremental (entre 1 y el máximo de int).
  std::size_t episode_size = 5000;
};

/// Aprende las reglas descriptivas del conjunto de ejemplos.
///
/// Lanza std::invalid_argument si el conjunto de ejemplos no es coherente (tamaños distintos,
/// variables sin etiquetas o clases fuera de rango) o el tamaño del episodio no es válido.
///
/// Usa el generador de números aleatorios de la biblioteca de C (srand y rand): no se puede llamar
/// a la vez desde varios hilos, y reinicia el generador del programa que lo llama.
std::vector<Rule> learn(const Dataset& data, const Options& options = {});

}  // namespace inslv
