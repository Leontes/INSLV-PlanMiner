// SPDX-License-Identifier: GPL-3.0-only
// Una población de individuos enteros, con valores de 0 a rango - 1.
#pragma once

#include <vector>

#include "inslv/genetico/poblacion.h"

namespace inslv {

class poblacionentera : public poblacion<std::vector<int>> {
 public:
  /// n individuos de tama valores de 0 a rango - 1, que mutan con probabilidad mut.
  poblacionentera(int rango, double mut, int n, int tama);

  /// Los valores del individuo i, todos a i % rango (sin azar).
  void PoblacionInicialAleatoria();

  /// Cambia cada valor de los dos últimos individuos, con la probabilidad de mutación, por otro
  /// al azar distinto.
  void MutacionUniforme_Estacionario();

 private:
  double prob_mutacion_;
  int rango_;
};

}  // namespace inslv
