// SPDX-License-Identifier: GPL-3.0-only
// El azar del algoritmo genético.
#pragma once

namespace inslv::genetico {

/// Un número al azar de 0 a RAND_MAX: el rand() global del original, hasta que haya un generador
/// propio (fase B). Las reglas dependen de cuántos se piden y en qué orden.
int aleatorio();

/// Cierto con probabilidad x. Con x = 1 siempre es cierto, pero pide un número igualmente.
[[nodiscard]] bool con_probabilidad(double x);

/// Un individuo al azar de 0 a n - 1 que no sea menos_este ni menor que minimo.
[[nodiscard]] int individuo_al_azar(int n, int menos_este, int minimo);

/// Dos puntos al azar de 0 a n - 1, con a <= b.
void dos_puntos_de_corte(int n, int& a, int& b);

}  // namespace inslv::genetico
