// SPDX-License-Identifier: GPL-3.0-only
// Una población de individuos enteros, con valores de 0 a rango - 1.
#pragma once

#include <vector>

#include "inslv/genetico/poblacion.hpp"

namespace inslv::genetico {

class PoblacionEntera : public Poblacion<std::vector<int>> {
 public:
  /// n individuos de tama valores de 0 a rango - 1, que mutan con probabilidad mut.
  PoblacionEntera(int rango, double mut, int n, int tama);

  /// Los valores del individuo i, todos a i % rango (sin azar).
  void inicializar();

  /// Cambia cada valor de los dos últimos individuos, con la probabilidad de mutación, por otro
  /// al azar distinto.
  void mutar();

 private:
  double prob_mutacion_;
  int rango_;
};

}  // namespace inslv::genetico
