// SPDX-License-Identifier: GPL-3.0-only
// El código genético de un individuo o de una regla. Parte de NSLV, de Raúl Pérez (2002).
#pragma once

#include <string>
#include <vector>

namespace inslv::genetico {

struct Codigo {
  /// El nivel de valor: las etiquetas que admite cada atributo, seguidas, con '0' y '1'.
  std::string etiquetas;
  /// El consecuente.
  int clase = 0;
  /// El nivel de variable: el de cada atributo y, al final, el umbral. Un atributo solo cuenta si
  /// su nivel llega al umbral.
  std::vector<double> niveles;
};

}  // namespace inslv::genetico
