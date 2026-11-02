// SPDX-License-Identifier: GPL-3.0-only
// El dominio de una variable: sus etiquetas.
#pragma once

#include <string>
#include <vector>

#include "inslv/difuso/fuzzy_t.h"

namespace inslv {

class domain_t {
 public:
  domain_t() = default;
  explicit domain_t(std::vector<fuzzy_t> etiquetas);

  [[nodiscard]] int N_etiquetas() const;
  /// La adaptación del valor a la etiqueta. Lanza std::logic_error si no existe.
  [[nodiscard]] double Adaptacion(double x, int etiqueta) const;
  /// La adaptación del valor a un conjunto de etiquetas, dado como una cadena de 0 y 1 (una por
  /// etiqueta): la de la primera a la que se adapta el valor o, si se adapta a otra más, 1 (supone
  /// que las etiquetas forman una partición). Lanza std::logic_error si el dominio está vacío o la
  /// cadena es más larga que las etiquetas.
  [[nodiscard]] double Adaptacion(double x, const std::string& etiquetas) const;
  /// Las etiquetas, a la depuración.
  void Pinta() const;
  /// El nombre de la etiqueta i.
  [[nodiscard]] std::string SPrint(int i) const;

 private:
  std::vector<fuzzy_t> label_;
};

}  // namespace inslv
