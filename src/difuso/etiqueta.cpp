// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/etiqueta.hpp"

#include <string>
#include <utility>

#include "inslv/comun/progreso.hpp"

namespace inslv::difuso {

Etiqueta::Etiqueta(double a, double b, double c, double d, std::string nombre)
    : a_(a), b_(b), c_(c), d_(d), nombre_(std::move(nombre)) {}

double Etiqueta::adaptacion(double x) const {
  if (x == -999999999) {
    return 1;
  }
  if (x < a_) {
    return 0;
  }
  if (x < b_) {
    return (x - a_) / (b_ - a_);
  }
  if (x <= c_) {
    return 1;
  }
  if (x < d_) {
    return (d_ - x) / (d_ - c_);
  }
  return 0;
}

void Etiqueta::pintar() const {
  comun::progreso(Verbosidad::depuracion)
      << "Nombre: " << nombre_ << '\n'
      << "[" << a_ << "," << b_ << "," << c_ << "," << d_ << "]\n";
}

const std::string& Etiqueta::nombre() const { return nombre_; }

}  // namespace inslv::difuso
