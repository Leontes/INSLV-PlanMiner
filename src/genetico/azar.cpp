// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/azar.hpp"

#include <cstdlib>
#include <utility>

namespace inslv::genetico {

int aleatorio() {
  return std::rand();  // NOLINT(concurrency-mt-unsafe)
}

bool con_probabilidad(double x) { return 1.0 * aleatorio() / (RAND_MAX * 1.0) <= x; }

int individuo_al_azar(int n, int menos_este, int minimo) {
  const double nd = n;
  int a = 0;
  do {
    a = static_cast<int>(nd * aleatorio() / (RAND_MAX + 1.0));
  } while (a == menos_este || a < minimo);
  return a;
}

void dos_puntos_de_corte(int n, int& a, int& b) {
  const double nd = n;
  a = static_cast<int>(nd * aleatorio() / (RAND_MAX + 1.0));
  b = static_cast<int>(nd * aleatorio() / (RAND_MAX + 1.0));
  if (a > b) {
    std::swap(a, b);
  }
}

}  // namespace inslv::genetico
