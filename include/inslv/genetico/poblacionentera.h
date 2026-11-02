// SPDX-License-Identifier: GPL-3.0-only
// Una población de individuos enteros, con valores de 0 a rango - 1.
#pragma once

#include "inslv/genetico/poblacion.h"

namespace inslv {

class poblacionentera : public poblacion<int> {
 public:
  /// n individuos de tama valores de 0 a rango - 1, que mutan con probabilidad mut.
  poblacionentera(int rango, double mut, int n, int tama);

  /// Los valores del individuo i, todos a i % rango (sin azar).
  void PoblacionInicialAleatoria();
  /// Todos los valores, a consecuente.
  void PoblacionInicialAleatoria(int consecuente);

  /// Cambia cada valor de los dos últimos individuos, con la probabilidad de mutación, por otro
  /// al azar distinto.
  void MutacionUniforme_Estacionario();
  /// Con probabilidad prob0, un valor al azar de cada uno de los dos últimos individuos: a 0 si
  /// no lo era y, si lo era, a uno al azar. Nada si rango <= 1.
  void MutacionUniformeMod_Estacionario(double prob0);
  /// En cada uno de los dos últimos individuos, pone a 0 un valor al azar que no lo sea, si lo
  /// encuentra en cinco intentos (si no, pide un sexto número, que no usa). Nada si rango <= 1.
  void MutacionUniformeMod_Estacionario_Especial();

 private:
  double prob_mutacion_;
  int rango_;
};

}  // namespace inslv
