// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacion.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

template <typename Cromosoma>
poblacion<Cromosoma>::poblacion(int n, int tama)
    : individuos_(indice(n), Cromosoma(indice(tama), typename Cromosoma::value_type{})) {}

template <typename Cromosoma>
Cromosoma& poblacion<Cromosoma>::Individuo(int i) {
  return individuos_[indice(i)];
}

template <typename Cromosoma>
const Cromosoma& poblacion<Cromosoma>::Individuo(int i) const {
  return individuos_[indice(i)];
}

template <typename Cromosoma>
void poblacion<Cromosoma>::Pasar(int i, int j) {
  individuos_[indice(i)] = individuos_[indice(j)];
}

template <typename Cromosoma>
void poblacion<Cromosoma>::Swap(int i, int j) {
  std::swap(individuos_[indice(i)], individuos_[indice(j)]);
}

template <typename Cromosoma>
void poblacion<Cromosoma>::Cruce2puntos_Estacionario(int indiv1, int indiv2) {
  Cromosoma& uno = individuos_[indice(indiv1)];
  Cromosoma& otro = individuos_[indice(indiv2)];
  int p1 = 0;
  int p2 = 0;
  PuntoCorte2(static_cast<int>(uno.size()), p1, p2);
  std::swap_ranges(uno.begin() + p1, uno.begin() + p2, otro.begin() + p1);
}

template class poblacion<std::string>;
template class poblacion<std::vector<int>>;
template class poblacion<std::vector<double>>;

}  // namespace inslv
