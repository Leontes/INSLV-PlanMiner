// SPDX-License-Identifier: GPL-3.0-only
// Una población de individuos reales. Parte de NSLV, de Raúl Pérez (2002).
#pragma once

#include "inslv/genetico/poblacion.h"

namespace inslv {

/// En la que usa el aprendizaje, el último valor de cada individuo es un umbral y los demás, uno
/// por atributo, se comparan con él.
class poblacionreal : public poblacion<double> {
 public:
  /// n individuos de tama valores, que mutan con probabilidad mut.
  poblacionreal(double mut, int n, int tama);

  /// Cada individuo i, para la clase c = i % clases: el valor j, I[j][c + 1] o, si es el mismo
  /// para todos los j, uno al azar entre 0 y 1; y el umbral, a 0,2.
  void PoblacionInicial(const double* const* I, int clases);

  /// Con la probabilidad de mutación, cada valor de los dos últimos individuos (salvo el umbral)
  /// pasa a estar justo al otro lado del umbral (a 0,01), sin salirse de [0, 1].
  void MutacionUniforme_Estacionario();

 private:
  double prob_mutacion_;
};

}  // namespace inslv
