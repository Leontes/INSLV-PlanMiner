// SPDX-License-Identifier: GPL-3.0-only
// Lo que el algoritmo genético le pide al azar. Las reglas dependen de cuántos números se piden y
// en qué orden.
#pragma once

#include "inslv/generador_de_azar.hpp"

namespace inslv::genetico {

/// Cierto con probabilidad x. Con x = 1 siempre es cierto, pero pide un número igualmente.
[[nodiscard]] bool con_probabilidad(GeneradorDeAzar& azar, double x);

/// Un individuo al azar de 0 a n - 1 que no sea menos_este ni menor que minimo.
[[nodiscard]] int individuo_al_azar(GeneradorDeAzar& azar, int n, int menos_este, int minimo);

/// Dos puntos al azar de 0 a n - 1, con a <= b.
void dos_puntos_de_corte(GeneradorDeAzar& azar, int n, int& a, int& b);

}  // namespace inslv::genetico
