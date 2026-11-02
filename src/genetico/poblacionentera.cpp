// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacionentera.h"

#include <cstddef>
#include <vector>

#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

// Una posición al azar de un individuo.
std::size_t PosicionAleatoria(const std::vector<int>& individuo) {
  return static_cast<std::size_t>(Aleatorio() % static_cast<int>(individuo.size()));
}

}  // namespace

poblacionentera::poblacionentera(int rango, double mut, int n, int tama)
    : poblacion<int>(n, tama), prob_mutacion_(mut), rango_(rango) {}

void poblacionentera::PoblacionInicialAleatoria() {
  for (std::size_t i = 0; i < individuos_.size(); ++i) {
    individuos_[i].assign(individuos_[i].size(), static_cast<int>(i) % rango_);
  }
}

void poblacionentera::PoblacionInicialAleatoria(int consecuente) {
  for (std::vector<int>& individuo : individuos_) {
    individuo.assign(individuo.size(), consecuente);
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

void poblacionentera::MutacionUniformeMod_Estacionario(double prob0) {
  if (rango_ <= 1) {
    return;
  }
  for (std::size_t i = individuos_.size() - 2; i < individuos_.size(); ++i) {
    if (Probabilidad(prob0)) {
      std::vector<int>& individuo = individuos_[i];
      int& valor = individuo[PosicionAleatoria(individuo)];
      valor = valor != 0 ? 0 : Aleatorio() % rango_;
    }
  }
}

void poblacionentera::MutacionUniformeMod_Estacionario_Especial() {
  if (rango_ <= 1) {
    return;
  }
  for (std::size_t i = individuos_.size() - 2; i < individuos_.size(); ++i) {
    std::vector<int>& individuo = individuos_[i];
    int intentos = 0;
    std::size_t j = PosicionAleatoria(individuo);
    while (intentos < 5 && individuo[j] == 0) {
      ++intentos;
      j = PosicionAleatoria(individuo);
    }
    // En el original, si lo encontraba y era 0, lo cambiaba por uno al azar, pero si lo
    // encuentra nunca es 0.
    if (intentos < 5) {
      individuo[j] = 0;
    }
  }
}

}  // namespace inslv
