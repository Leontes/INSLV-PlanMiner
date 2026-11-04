// SPDX-License-Identifier: GPL-3.0-only
// Una etiqueta lingüística: un conjunto difuso con nombre.
#pragma once

#include <string>

namespace inslv::difuso {

/// Un trapecio: a <= b <= c <= d.
class Etiqueta {
 public:
  Etiqueta(double a, double b, double c, double d, std::string nombre);

  /// La adaptación del valor. Un desconocido (-999999999) se adapta del todo.
  [[nodiscard]] double adaptacion(double x) const;
  /// La definición, a la depuración.
  void pintar() const;
  [[nodiscard]] const std::string& nombre() const;

 private:
  double a_;
  double b_;
  double c_;
  double d_;
  std::string nombre_;
};

}  // namespace inslv::difuso
