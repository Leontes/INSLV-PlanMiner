// SPDX-License-Identifier: GPL-3.0-only
// Lo común a las poblaciones binaria, entera y real.
#pragma once

#include <string>
#include <vector>

namespace inslv {

/// Una población de n individuos del mismo tamaño. Es la de un genético estacionario: los
/// operadores cambian solo los dos últimos individuos.
template <typename Cromosoma>
class poblacion {
 public:
  /// n individuos de tama valores, a 0 hasta que se inicializan.
  poblacion(int n, int tama);

  [[nodiscard]] Cromosoma& Individuo(int i);
  [[nodiscard]] const Cromosoma& Individuo(int i) const;
  /// Copia el individuo j en el i.
  void Pasar(int i, int j);
  void Swap(int i, int j);

  /// Intercambia entre indiv1 e indiv2 los valores de dos puntos al azar (el segundo, sin
  /// incluir).
  void Cruce2puntos_Estacionario(int indiv1, int indiv2);

 protected:
  std::vector<Cromosoma> individuos_;
};

extern template class poblacion<std::string>;
extern template class poblacion<std::vector<int>>;
extern template class poblacion<std::vector<double>>;

}  // namespace inslv
