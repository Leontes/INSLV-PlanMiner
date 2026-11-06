// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/azar.hpp"

#include <utility>

namespace inslv::genetico {

namespace {

// Un entero de 0 a n - 1. Con estas operaciones y en este orden, como el original.
int menor_que(GeneradorDeAzar& azar, int n) {
  const double nd = n;
  return static_cast<int>(nd * azar() / (GeneradorDeAzar::maximo + 1.0));
}

}  // namespace

bool con_probabilidad(GeneradorDeAzar& azar, double x) {
  return 1.0 * azar() / (GeneradorDeAzar::maximo * 1.0) <= x;
}

int individuo_al_azar(GeneradorDeAzar& azar, int n, int menos_este, int minimo) {
  int a = 0;
  do {
    a = menor_que(azar, n);
  } while (a == menos_este || a < minimo);
  return a;
}

void dos_puntos_de_corte(GeneradorDeAzar& azar, int n, int& a, int& b) {
  a = menor_que(azar, n);
  b = menor_que(azar, n);
  if (a > b) {
    std::swap(a, b);
  }
}

}  // namespace inslv::genetico
