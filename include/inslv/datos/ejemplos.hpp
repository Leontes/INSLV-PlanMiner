// SPDX-License-Identifier: GPL-3.0-only
// Los ejemplos con los que se aprende.
#pragma once

#include <vector>

#include "inslv/datos/ejemplo.hpp"

namespace inslv::datos {

/// Un conjunto de ejemplos: los valores de cada uno (la clase, en la columna de la variable
/// consecuente, como el centro de su etiqueta; un desconocido, como -999999999) y lo que anota el
/// aprendizaje de cada uno al clasificarlo con las reglas (anotar): si está bien clasificado, con
/// qué grado y peso lo cubren la mejor regla de su clase y la de otra, qué regla lo clasifica y si
/// es la única que lo hace.
class Ejemplos {
 public:
  Ejemplos() = default;
  /// ejemplos ejemplos de variables valores, a 0. Lanza std::logic_error si ejemplos < 0 o
  /// variables <= 0.
  Ejemplos(int ejemplos, int variables);
  /// Por columnas: columnas[variable][ejemplo].
  explicit Ejemplos(const std::vector<std::vector<double>>& columnas);

  [[nodiscard]] double valor(int ejemplo, int variable) const;
  [[nodiscard]] Ejemplo ejemplo(int i) const;
  [[nodiscard]] int numero_de_ejemplos() const;

  /// Cuántos ejemplos no están bien clasificados.
  [[nodiscard]] int sin_clasificar_bien() const;
  /// Cuántos ejemplos de cada clase no están bien clasificados.
  [[nodiscard]] std::vector<int> sin_clasificar_bien_por_clase(int variable_clase,
                                                               int clases) const;

  /// Anota lo que da clasificar el ejemplo i (ver la clase).
  void anotar(int i, bool bien_clasificado, double grado_p, double peso_p, double grado_n,
              double peso_n, int regla, int aislada);
  [[nodiscard]] bool bien_clasificado(int i) const;
  [[nodiscard]] double grado_positivo(int i) const;
  [[nodiscard]] double grado_negativo(int i) const;
  [[nodiscard]] double peso_positivo(int i) const;
  [[nodiscard]] double peso_negativo(int i) const;
  [[nodiscard]] int clase(int variable_clase, int i) const;
  [[nodiscard]] int regla_usada(int i) const;
  [[nodiscard]] int regla_aislada(int i) const;

  /// Los n primeros ejemplos, sin las anotaciones.
  [[nodiscard]] Ejemplos primeros(int n) const;
  /// Los ejemplos de ini a fin (incluido), sin las anotaciones.
  [[nodiscard]] Ejemplos subconjunto(int ini, int fin) const;

 private:
  int n_variables_ = 0;
  std::vector<std::vector<double>> valores_;  // valores_[ejemplo][variable]
  std::vector<bool> bien_clasificado_;
  std::vector<double> grado_positivo_;
  std::vector<double> peso_positivo_;
  std::vector<double> grado_negativo_;
  std::vector<double> peso_negativo_;
  std::vector<int> regla_usada_;
  std::vector<int> regla_aislada_;
};

}  // namespace inslv::datos
