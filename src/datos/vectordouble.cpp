// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/datos/vectordouble.h"

#include <cstddef>
#include <utility>

#include "inslv/comun/progreso.hpp"

namespace inslv {

vectordouble::vectordouble(int tamano) {
  if (tamano > 0) {
    valores_.reserve(static_cast<std::size_t>(tamano));
  }
}

vectordouble::vectordouble(std::vector<double> valores) : valores_(std::move(valores)) {}

double vectordouble::At(int pos) const {
  // En el original se comprobaba pos > numero: At(numero) leía fuera del vector (nunca pasa).
  if (pos < 0 || static_cast<std::size_t>(pos) >= valores_.size()) {
    progreso(Verbosidad::depuracion) << "No existe esa posicion" << pos << '\n';
    return -999999999;
  }
  return valores_[static_cast<std::size_t>(pos)];
}

}  // namespace inslv
