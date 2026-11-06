// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/dominio.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace inslv::difuso {

Dominio::Dominio(std::vector<Etiqueta> etiquetas) : etiquetas_(std::move(etiquetas)) {}

int Dominio::numero_de_etiquetas() const { return static_cast<int>(etiquetas_.size()); }

double Dominio::adaptacion(double x, int etiqueta) const {
  if (etiquetas_.empty()) {
    throw std::logic_error("inslv: el dominio no está creado");
  }
  if (etiqueta < 0 || static_cast<std::size_t>(etiqueta) >= etiquetas_.size()) {
    throw std::logic_error("inslv: la etiqueta " + std::to_string(etiqueta) +
                           " no está en el dominio");
  }
  return etiquetas_[static_cast<std::size_t>(etiqueta)].adaptacion(x);
}

double Dominio::adaptacion(double x, std::string_view etiquetas) const {
  if (etiquetas_.empty()) {
    throw std::logic_error("inslv: el dominio no está creado");
  }
  if (etiquetas.length() > etiquetas_.size()) {
    throw std::logic_error("inslv: hay más etiquetas que las del dominio");
  }
  double mayor = 0;
  for (std::size_t i = 0; i < etiquetas.length() && mayor < 1; ++i) {
    if (etiquetas[i] == '1') {
      if (mayor == 0) {
        mayor = etiquetas_[i].adaptacion(x);
      } else if (etiquetas_[i].adaptacion(x) > 0) {
        mayor = 1;
      }
    }
  }
  return mayor;
}

void Dominio::pintar() const {
  for (const Etiqueta& etiqueta : etiquetas_) {
    etiqueta.pintar();
  }
}

const std::string& Dominio::nombre_de_etiqueta(int i) const {
  return etiquetas_[static_cast<std::size_t>(i)].nombre();
}

}  // namespace inslv::difuso
