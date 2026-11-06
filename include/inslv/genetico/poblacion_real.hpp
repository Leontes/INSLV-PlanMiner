// SPDX-License-Identifier: GPL-3.0-only
// Una población de individuos reales. Parte de NSLV, de Raúl Pérez (2002).
#pragma once

#include <vector>

#include "inslv/genetico/poblacion.hpp"

namespace inslv::genetico {

/// En la que usa el aprendizaje, el último valor de cada individuo es un umbral y los demás, uno
/// por atributo, se comparan con él.
class PoblacionReal : public Poblacion<std::vector<double>> {
 public:
  /// n individuos de tama valores, que mutan con probabilidad mut.
  PoblacionReal(double mut, int n, int tama);

  /// Cada individuo i, para la clase c = i % clases: el valor j, I[j][c + 1] o, si es el mismo
  /// para todos los j, uno al azar entre 0 y 1; y el umbral, a 0,2.
  void inicializar(GeneradorDeAzar& azar, const std::vector<std::vector<double>>& I, int clases);

  /// Con la probabilidad de mutación, cada valor de los dos últimos individuos (salvo el umbral)
  /// pasa a estar justo al otro lado del umbral (a 0,01), sin salirse de [0, 1].
  void mutar(GeneradorDeAzar& azar);

 private:
  double prob_mutacion_;
};

}  // namespace inslv::genetico
