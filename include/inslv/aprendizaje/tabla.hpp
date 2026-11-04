// SPDX-License-Identifier: GPL-3.0-only
// La tabla con las adaptaciones de los ejemplos a las etiquetas de las variables.
#pragma once

#include <string>
#include <vector>

#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"

namespace inslv::aprendizaje {

/// Se calcula una vez para unos ejemplos y evita repetir la cuenta al valorar cada individuo.
class TablaDeAdaptaciones {
 public:
  /// Lanza std::invalid_argument si algún valor no se adapta a ninguna etiqueta de su variable.
  TablaDeAdaptaciones(const difuso::Variables& variables, const datos::Ejemplos& ejemplos);

  /// La adaptación del ejemplo a una etiqueta de la variable.
  [[nodiscard]] double adaptacion_a_etiqueta(int ejemplo, int variable, int etiqueta) const;
  /// La adaptación del ejemplo a las etiquetas de la variable marcadas con '1' a partir de
  /// etiquetas: la de la primera con adaptación positiva o, si son dos, 1.
  [[nodiscard]] double adaptacion_a_variable(int ejemplo, int variable,
                                             const char* etiquetas) const;
  /// La adaptación del ejemplo a un antecedente (ver genetico::Codigo): la menor de los atributos
  /// con nivel por encima del umbral.
  [[nodiscard]] double adaptacion_al_antecedente(int ejemplo, const std::string& etiquetas,
                                                 const std::vector<double>& niveles) const;
  /// En pos, la adaptación del ejemplo a una etiqueta del consecuente; en neg, 0 si pos es 1 y 1 si
  /// no.
  void consecuente(int ejemplo, int etiqueta, double& pos, double& neg) const;
  /// La primera etiqueta de la variable con la mayor adaptación del ejemplo.
  [[nodiscard]] int mejor_etiqueta(int ejemplo, int variable) const;

 private:
  // adaptaciones_[variable][etiqueta][ejemplo]
  std::vector<std::vector<std::vector<double>>> adaptaciones_;
  int n_antecedente_;
  int consecuente_;
};

}  // namespace inslv::aprendizaje
