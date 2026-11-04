// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacion.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "inslv/genetico/azar.hpp"

namespace inslv::genetico {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

template <typename Cromosoma>
Poblacion<Cromosoma>::Poblacion(int n, int tama)
    : individuos_(indice(n), Cromosoma(indice(tama), typename Cromosoma::value_type{})) {}

template <typename Cromosoma>
Cromosoma& Poblacion<Cromosoma>::individuo(int i) {
  return individuos_[indice(i)];
}

template <typename Cromosoma>
const Cromosoma& Poblacion<Cromosoma>::individuo(int i) const {
  return individuos_[indice(i)];
}

template <typename Cromosoma>
void Poblacion<Cromosoma>::copiar(int i, int j) {
  individuos_[indice(i)] = individuos_[indice(j)];
}

template <typename Cromosoma>
void Poblacion<Cromosoma>::intercambiar(int i, int j) {
  std::swap(individuos_[indice(i)], individuos_[indice(j)]);
}

template <typename Cromosoma>
void Poblacion<Cromosoma>::cruzar_en_dos_puntos(int indiv1, int indiv2) {
  Cromosoma& uno = individuos_[indice(indiv1)];
  Cromosoma& otro = individuos_[indice(indiv2)];
  int p1 = 0;
  int p2 = 0;
  dos_puntos_de_corte(static_cast<int>(uno.size()), p1, p2);
  std::swap_ranges(uno.begin() + p1, uno.begin() + p2, otro.begin() + p1);
}

template class Poblacion<std::string>;
template class Poblacion<std::vector<int>>;
template class Poblacion<std::vector<double>>;

}  // namespace inslv::genetico
