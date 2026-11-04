// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacion_entera.hpp"

#include <cstddef>
#include <vector>

#include "inslv/genetico/azar.hpp"

namespace inslv::genetico {

PoblacionEntera::PoblacionEntera(int rango, double mut, int n, int tama)
    : Poblacion<std::vector<int>>(n, tama), prob_mutacion_(mut), rango_(rango) {}

void PoblacionEntera::inicializar() {
  for (std::size_t i = 0; i < individuos_.size(); ++i) {
    individuos_[i].assign(individuos_[i].size(), static_cast<int>(i) % rango_);
  }
}

void PoblacionEntera::mutar() {
  for (std::size_t i = individuos_.size() - 2; i < individuos_.size(); ++i) {
    for (int& valor : individuos_[i]) {
      if (con_probabilidad(prob_mutacion_)) {
        int nuevo = 0;
        do {
          nuevo = aleatorio() % rango_;
        } while (nuevo == valor);
        valor = nuevo;
      }
    }
  }
}

}  // namespace inslv::genetico
