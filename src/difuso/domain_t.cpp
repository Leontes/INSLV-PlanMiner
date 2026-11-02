// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/domain_t.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace inslv {

domain_t::domain_t(std::vector<fuzzy_t> etiquetas) : label_(std::move(etiquetas)) {}

int domain_t::N_etiquetas() const { return static_cast<int>(label_.size()); }

double domain_t::Adaptacion(double x, int etiqueta) const {
  if (label_.empty()) {
    throw std::logic_error("inslv: el dominio no está creado");
  }
  if (etiqueta < 0 || static_cast<std::size_t>(etiqueta) >= label_.size()) {
    throw std::logic_error("inslv: la etiqueta " + std::to_string(etiqueta) +
                           " no está en el dominio");
  }
  return label_[static_cast<std::size_t>(etiqueta)].Adaptacion(x);
}

double domain_t::Adaptacion(double x, const std::string& etiquetas) const {
  if (label_.empty()) {
    throw std::logic_error("inslv: el dominio no está creado");
  }
  if (etiquetas.length() > label_.size()) {
    throw std::logic_error("inslv: hay más etiquetas que las del dominio");
  }
  double mayor = 0;
  for (std::size_t i = 0; i < etiquetas.length() && mayor < 1; ++i) {
    if (etiquetas[i] == '1') {
      if (mayor == 0) {
        mayor = label_[i].Adaptacion(x);
      } else if (label_[i].Adaptacion(x) > 0) {
        mayor = 1;
      }
    }
  }
  return mayor;
}

void domain_t::Pinta() const {
  for (const fuzzy_t& etiqueta : label_) {
    etiqueta.Pinta();
  }
}

std::string domain_t::SPrint(int i) const { return label_[static_cast<std::size_t>(i)].SPrint(); }

}  // namespace inslv
