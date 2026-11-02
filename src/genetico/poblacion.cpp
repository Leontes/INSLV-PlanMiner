// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/poblacion.h"

#include <algorithm>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

template <typename T>
poblacion<T>::poblacion(int n, int tama) : individuos_(indice(n), std::vector<T>(indice(tama))) {}

template <typename T>
std::span<T> poblacion<T>::Individuo(int i) {
  return individuos_[indice(i)];
}

template <typename T>
std::span<const T> poblacion<T>::Individuo(int i) const {
  return individuos_[indice(i)];
}

template <typename T>
T poblacion<T>::GetValue(int indiv, int bit) const {
  return individuos_[indice(indiv)][indice(bit)];
}

template <typename T>
void poblacion<T>::PutValue(int indiv, int bit, T value) {
  individuos_[indice(indiv)][indice(bit)] = value;
}

template <typename T>
void poblacion<T>::Pasar(int i, int j) {
  individuos_[indice(i)] = individuos_[indice(j)];
}

template <typename T>
void poblacion<T>::Swap(int i, int j) {
  std::swap(individuos_[indice(i)], individuos_[indice(j)]);
}

template <typename T>
void poblacion<T>::Cruce2puntos_Estacionario(int indiv1, int indiv2) {
  std::vector<T>& uno = individuos_[indice(indiv1)];
  std::vector<T>& otro = individuos_[indice(indiv2)];
  int p1 = 0;
  int p2 = 0;
  PuntoCorte2(static_cast<int>(uno.size()), p1, p2);
  std::swap_ranges(uno.begin() + p1, uno.begin() + p2, otro.begin() + p1);
}

template class poblacion<char>;
template class poblacion<int>;
template class poblacion<double>;

}  // namespace inslv
