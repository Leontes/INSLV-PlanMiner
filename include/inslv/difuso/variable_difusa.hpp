// SPDX-License-Identifier: GPL-3.0-only
// Una variable: un nombre y sus etiquetas.
#pragma once

#include <string>
#include <vector>

#include "inslv/difuso/dominio.hpp"
#include "inslv/difuso/etiqueta.hpp"

namespace inslv::difuso {

class VariableDifusa {
 public:
  VariableDifusa(std::string nombre, std::vector<Etiqueta> etiquetas);

  [[nodiscard]] const std::string& nombre() const;
  [[nodiscard]] const Dominio& dominio() const;
  /// La variable con su dominio, a la depuración.
  void pintar() const;

 private:
  std::string nombre_;
  Dominio dominio_;
};

}  // namespace inslv::difuso
