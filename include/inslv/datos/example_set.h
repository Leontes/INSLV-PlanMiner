// SPDX-License-Identifier: GPL-3.0-only
// Los ejemplos con los que se aprende.
#pragma once

#include <vector>

#include "inslv/datos/vectordouble.h"

namespace inslv {

/// Un conjunto de ejemplos: los valores de cada uno (la clase, en la columna de la variable
/// consecuente, como el centro de su etiqueta; un desconocido, como -999999999) y lo que anota el
/// aprendizaje de cada uno al clasificarlo con las reglas (Mark): si está bien clasificado, con
/// qué grado y peso lo cubren la mejor regla de su clase y la de otra, qué regla lo clasifica y si
/// es la única que lo hace.
class example_set {
 public:
  example_set() = default;
  /// ejemplos ejemplos de variables valores, a 0. Lanza std::logic_error si ejemplos < 0 o
  /// variables <= 0.
  example_set(int ejemplos, int variables);
  /// Por columnas: columnas[variable][ejemplo].
  explicit example_set(const std::vector<std::vector<double>>& columnas);

  [[nodiscard]] double Data(int ejemplo, int variable) const;
  [[nodiscard]] vectordouble Data(int ejemplo) const;
  [[nodiscard]] int N_Examples() const;

  /// Cuántos ejemplos no están bien clasificados (Mark).
  [[nodiscard]] int Not_Covered_Examples() const;
  /// Cuántos ejemplos de cada clase no están bien clasificados, en n_examples_in_class[clase].
  void Examples_per_Class(int VarClass, int nclasses, int* n_examples_in_class) const;

  /// Anota lo que da clasificar los tama primeros ejemplos (ver la clase).
  void Mark(const int* v, int tama, const double* grado_p, const double* peso_p,
            const double* grado_n, const double* peso_n, const int* rule, const int* r_aislada);
  void UnMarkAll();
  [[nodiscard]] bool Is_Covered(int i) const;
  [[nodiscard]] double Grade_Is_Positive_Covered(int i) const;
  [[nodiscard]] double Grade_Is_Negative_Covered(int i) const;
  [[nodiscard]] double Peso_Is_Positive_Covered(int i) const;
  [[nodiscard]] double Peso_Is_Negative_Covered(int i) const;
  [[nodiscard]] int Class_of_example(int VarClase, int i) const;
  [[nodiscard]] int Rule_Used(int i) const;
  [[nodiscard]] int Rule_Aislada(int i) const;

  /// Los tama primeros ejemplos, o los de ini a fin (incluido).
  [[nodiscard]] example_set SubSet(int tama) const;
  [[nodiscard]] example_set SubSet(int ini, int fin) const;

 private:
  // Los que cumplen la condición, sin las anotaciones.
  template <typename Condicion>
  [[nodiscard]] example_set Seleccionar(Condicion condicion) const;

  int n_example = 0;
  int n_variable = 0;
  std::vector<std::vector<double>> data;  // data[ejemplo][variable]
  std::vector<bool> covered;
  std::vector<double> g_positive_covered;
  std::vector<double> peso_positive;
  std::vector<double> g_negative_covered;
  std::vector<double> peso_negative;
  std::vector<int> rule_used;
  std::vector<int> rule_aislada;
};

}  // namespace inslv
