// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/datos/ejemplo.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace inslv::datos {

Ejemplo::Ejemplo(std::vector<double> valores) : valores_(std::move(valores)) {}

double Ejemplo::valor(int pos) const {
  if (pos < 0 || static_cast<std::size_t>(pos) >= valores_.size()) {
    return -999999999;
  }
  return valores_[static_cast<std::size_t>(pos)];
}

}  // namespace inslv::datos
