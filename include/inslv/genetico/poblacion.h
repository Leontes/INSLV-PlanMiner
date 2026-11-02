// SPDX-License-Identifier: GPL-3.0-only
// Lo común a las poblaciones binaria, entera y real.
#pragma once

#include <span>
#include <vector>

namespace inslv {

/// Una población de n individuos, cada uno con el mismo número de valores de tipo T. Es la de un
/// genético estacionario: los operadores cambian solo los dos últimos individuos.
template <typename T>
class poblacion {
 public:
  /// n individuos de tama valores, a 0 hasta que se inicializan.
  poblacion(int n, int tama);

  /// Los valores del individuo i (se pueden cambiar).
  [[nodiscard]] std::span<T> Individuo(int i);
  [[nodiscard]] std::span<const T> Individuo(int i) const;
  [[nodiscard]] T GetValue(int indiv, int bit) const;
  void PutValue(int indiv, int bit, T value);
  /// Copia el individuo j en el i.
  void Pasar(int i, int j);
  void Swap(int i, int j);

  /// Intercambia entre indiv1 e indiv2 los valores de dos puntos al azar (el segundo, sin
  /// incluir).
  void Cruce2puntos_Estacionario(int indiv1, int indiv2);

 protected:
  std::vector<std::vector<T>> individuos_;
};

extern template class poblacion<char>;
extern template class poblacion<int>;
extern template class poblacion<double>;

}  // namespace inslv
