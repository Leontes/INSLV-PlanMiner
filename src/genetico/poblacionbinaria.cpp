// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacionbinaria.h"

#include <cstddef>
#include <string>
#include <vector>

#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

char BitAleatorio() { return Probabilidad(0.9) ? '1' : '0'; }

}  // namespace

poblacionbinaria::poblacionbinaria(double mut, int n, int tama)
    : poblacion<std::string>(n, tama), prob_mutacion_(mut) {}

void poblacionbinaria::PoblacionInicial(const std::vector<std::vector<int>>& sujeto,
                                        const std::vector<int>& tama_dom) {
  for (std::size_t i = 0; i < individuos_.size(); ++i) {
    std::string& individuo = individuos_[i];
    std::size_t j = 0;
    for (std::size_t k = 0; k < tama_dom.size(); ++k) {
      const auto bits = static_cast<std::size_t>(tama_dom[k]);
      if (sujeto[i][k] == -1) {
        for (std::size_t l = 0; l < bits; ++l) {
          individuo[j + l] = BitAleatorio();
        }
      } else {
        for (std::size_t l = 0; l < bits; ++l) {
          individuo[j + l] = '0';
        }
        individuo[j + static_cast<std::size_t>(sujeto[i][k])] = '1';
      }
      j += bits;
    }
  }
}

void poblacionbinaria::MutacionUniforme_Estacionario() {
  for (std::size_t i = individuos_.size() - 2; i < individuos_.size(); ++i) {
    for (char& bit : individuos_[i]) {
      if (Probabilidad(prob_mutacion_)) {
        bit = bit == '1' ? '0' : '1';
      }
    }
  }
}

}  // namespace inslv
