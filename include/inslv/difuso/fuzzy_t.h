// SPDX-License-Identifier: GPL-3.0-only
// Una etiqueta difusa trapezoidal.
#pragma once

#include <string>

namespace inslv {

/// Etiqueta trapezoidal [a, b, c, d]: la adaptación de un valor es 1 en [b, c], sube de 0 a 1
/// entre a y b, baja de 1 a 0 entre c y d, y es 0 fuera. Un desconocido (-999999999) se adapta
/// por completo. (El original admitía etiquetas que se extendían hasta el infinito por un extremo,
/// pero nunca las creaba.)
class fuzzy_t {
 public:
  fuzzy_t() = default;
  fuzzy_t(double a, double b, double c, double d, std::string nombre);

  [[nodiscard]] double Adaptacion(double x) const;
  /// La definición, a la depuración.
  void Pinta() const;
  /// El nombre.
  [[nodiscard]] std::string SPrint() const;

 private:
  double a_ = 0;
  double b_ = 0;
  double c_ = 0;
  double d_ = 0;
  std::string nombre_;
};

}  // namespace inslv
