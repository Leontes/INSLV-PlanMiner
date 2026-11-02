// SPDX-License-Identifier: GPL-3.0-only
// El azar del algoritmo genético.
#pragma once

namespace inslv {

/// Un número al azar de 0 a RAND_MAX: el rand() global del original, hasta que haya un generador
/// propio (fase B). Las reglas dependen de cuántos se piden y en qué orden.
int Aleatorio();

/// Cierto con probabilidad x. Con x = 1 siempre es cierto, pero pide un número igualmente.
[[nodiscard]] bool Probabilidad(double x);

/// Un individuo al azar de 0 a n - 1 que no sea menos_este ni menor que eli.
[[nodiscard]] int Select_Random_Individuo(int n, int menos_este, int eli);

/// Dos puntos al azar de 0 a n - 1, con a <= b.
void PuntoCorte2(int n, int& a, int& b);

}  // namespace inslv
