// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/operaciones.h"

#include <cstdlib>
#include <utility>

namespace inslv {

int Aleatorio() {
  return std::rand();  // NOLINT(concurrency-mt-unsafe)
}

bool Probabilidad(double x) { return 1.0 * Aleatorio() / (RAND_MAX * 1.0) <= x; }

int Select_Random_Individuo(int n, int menos_este, int eli) {
  const double nd = n;
  int a = 0;
  do {
    a = static_cast<int>(nd * Aleatorio() / (RAND_MAX + 1.0));
  } while (a == menos_este || a < eli);
  return a;
}

void PuntoCorte2(int n, int& a, int& b) {
  const double nd = n;
  a = static_cast<int>(nd * Aleatorio() / (RAND_MAX + 1.0));
  b = static_cast<int>(nd * Aleatorio() / (RAND_MAX + 1.0));
  if (a > b) {
    std::swap(a, b);
  }
}

}  // namespace inslv
