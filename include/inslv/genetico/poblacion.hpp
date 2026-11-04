// SPDX-License-Identifier: GPL-3.0-only
// Lo común a las poblaciones binaria, entera y real.
#pragma once

#include <string>
#include <vector>

namespace inslv::genetico {

/// Una población de n individuos del mismo tamaño. Es la de un genético estacionario: los
/// operadores cambian solo los dos últimos individuos.
template <typename Cromosoma>
class Poblacion {
 public:
  /// n individuos de tama valores, a 0 hasta que se inicializan.
  Poblacion(int n, int tama);

  [[nodiscard]] Cromosoma& individuo(int i);
  [[nodiscard]] const Cromosoma& individuo(int i) const;
  /// Copia el individuo j en el i.
  void copiar(int i, int j);
  void intercambiar(int i, int j);

  /// Intercambia entre indiv1 e indiv2 los valores de dos puntos al azar (el segundo, sin
  /// incluir).
  void cruzar_en_dos_puntos(int indiv1, int indiv2);

 protected:
  std::vector<Cromosoma> individuos_;
};

extern template class Poblacion<std::string>;
extern template class Poblacion<std::vector<int>>;
extern template class Poblacion<std::vector<double>>;

}  // namespace inslv::genetico
