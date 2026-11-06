// SPDX-License-Identifier: GPL-3.0-only
// Las etiquetas de una variable.
#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "inslv/difuso/etiqueta.hpp"

namespace inslv::difuso {

class Dominio {
 public:
  Dominio() = default;
  explicit Dominio(std::vector<Etiqueta> etiquetas);

  [[nodiscard]] int numero_de_etiquetas() const;
  /// La adaptación del valor a la etiqueta. Lanza std::logic_error si no existe.
  [[nodiscard]] double adaptacion(double x, int etiqueta) const;
  /// La adaptación del valor a un conjunto de etiquetas, dado como una cadena de 0 y 1 (una por
  /// etiqueta): la de la primera a la que se adapta el valor o, si se adapta a otra más, 1 (supone
  /// que las etiquetas forman una partición). Lanza std::logic_error si el dominio está vacío o la
  /// cadena es más larga que las etiquetas.
  [[nodiscard]] double adaptacion(double x, std::string_view etiquetas) const;
  /// Las etiquetas, a la depuración.
  void pintar() const;
  [[nodiscard]] const std::string& nombre_de_etiqueta(int i) const;

 private:
  std::vector<Etiqueta> etiquetas_;
};

}  // namespace inslv::difuso
