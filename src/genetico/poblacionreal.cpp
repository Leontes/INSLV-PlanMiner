// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          poblacionreal.cpp  -  description
                             -------------------
    begin                : Tue Apr 30 2002
    copyright            : (C) 2002 by Raúl Pérez
    email                : fgr@decsai.ugr.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "inslv/genetico/poblacionreal.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <vector>

#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/operaciones.h"

namespace inslv {

poblacionreal::poblacionreal(double mut, int n, int tama)
    : poblacion<std::vector<double>>(n, tama), prob_mutacion_(mut) {}

void poblacionreal::PoblacionInicial(const std::vector<std::vector<double>>& I, int clases) {
  const std::size_t atributos = individuos_[0].size() - 1;
  progreso(Verbosidad::depuracion) << "CL " << atributos << '\n';
  // Si I[j][c + 1] es el mismo para todos los atributos j.
  std::vector<bool> constante;
  for (std::size_t c = 1; c <= static_cast<std::size_t>(clases); ++c) {
    double mayor = I[0][c];
    double menor = I[0][c];
    progreso(Verbosidad::depuracion) << "I " << I[0][c] << '\n';
    for (std::size_t j = 1; j < atributos; ++j) {
      mayor = std::max(mayor, I[j][c]);
      menor = std::min(menor, I[j][c]);
    }
    // En el original, con un solo atributo, mayor y menor se quedaban sin guardar y se
    // comparaban sin inicializar.
    constante.push_back(menor == mayor);
  }
  for (std::size_t i = 0; i < individuos_.size(); ++i) {
    const std::size_t c = i % static_cast<std::size_t>(clases);
    std::vector<double>& individuo = individuos_[i];
    for (std::size_t j = 0; j < atributos; ++j) {
      individuo[j] = constante[c] ? Aleatorio() / (RAND_MAX + 1.0) : I[j][c + 1];
    }
    individuo.back() = 0.2;
  }
}

void poblacionreal::MutacionUniforme_Estacionario() {
  for (std::size_t i = individuos_.size() - 2; i < individuos_.size(); ++i) {
    std::vector<double>& individuo = individuos_[i];
    const std::size_t umbral = individuo.size() - 1;
    for (std::size_t k = 0; k < umbral; ++k) {
      if (Probabilidad(prob_mutacion_)) {
        const double valor =
            individuo[k] < individuo[umbral] ? individuo[umbral] + 0.01 : individuo[umbral] - 0.01;
        individuo[k] = std::clamp(valor, 0.0, 1.0);
      }
    }
  }
}

}  // namespace inslv
