// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacionentera.h"

#include <cstddef>
#include <vector>

#include "inslv/genetico/operaciones.h"

namespace inslv {

poblacionentera::poblacionentera(int rango, double mut, int n, int tama)
    : poblacion<int>(n, tama), prob_mutacion_(mut), rango_(rango) {}

void poblacionentera::PoblacionInicialAleatoria() {
  for (std::size_t i = 0; i < individuos_.size(); ++i) {
    individuos_[i].assign(individuos_[i].size(), static_cast<int>(i) % rango_);
  }
}

void poblacionentera::MutacionUniforme_Estacionario() {
  for (std::size_t i = individuos_.size() - 2; i < individuos_.size(); ++i) {
    for (int& valor : individuos_[i]) {
      if (Probabilidad(prob_mutacion_)) {
        int nuevo = 0;
        do {
          nuevo = Aleatorio() % rango_;
        } while (nuevo == valor);
        valor = nuevo;
      }
    }
  }
}

}  // namespace inslv
