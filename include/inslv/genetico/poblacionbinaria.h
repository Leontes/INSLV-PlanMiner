// SPDX-License-Identifier: GPL-3.0-only
// Una población de individuos binarios: cadenas de '0' y '1'.
#pragma once

#include "inslv/genetico/poblacion.h"

namespace inslv {

class poblacionbinaria : public poblacion<char> {
 public:
  /// n individuos de tama bits, que mutan con probabilidad mut.
  poblacionbinaria(double mut, int n, int tama);

  /// Cada bit, a 1 con probabilidad 0,9.
  void PoblacionInicialAleatoria();
  /// Cada individuo i, por trozos: el k, de tama_dom[k] bits, todos a 0 salvo el sujeto[i][k] o,
  /// si sujeto[i][k] es -1, a 1 cada uno con probabilidad 0,9.
  void PoblacionInicial(const int* const* sujeto, int tama, const int* tama_dom);

  /// Cambia cada bit de los dos últimos individuos con la probabilidad de mutación.
  void MutacionUniforme_Estacionario();

 private:
  double prob_mutacion_;
};

}  // namespace inslv
