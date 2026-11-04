// SPDX-License-Identifier: GPL-3.0-only
// El algoritmo incremental de INSLV: aprende las reglas por episodios.
#pragma once

#include <vector>

#include "inslv/aprendizaje/tabla.h"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/multipoblacion.hpp"
#include "inslv/inslv.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv {

/// La proporción de ejemplos que clasifican bien las reglas. Anota en E lo que da cada uno
/// (datos::Ejemplos::Mark).
double Aciertos(const reglas::ConjuntoDeReglas& R, const difuso::Variables& V, datos::Ejemplos& E);
/// Las reglas que clasifican algún ejemplo, bien o mal.
[[nodiscard]] std::vector<int> Reglas_Que_Disparan(const reglas::ConjuntoDeReglas& R,
                                                   const datos::Ejemplos& E);
/// Actualiza con esos ejemplos el peso de las reglas modificadas. Usa el último individuo de G.
void ActualizarPeso(genetico::MultiPoblacion& G, const datos::Ejemplos& E,
                    reglas::ConjuntoDeReglas& R, const TablaDeAdaptaciones& tabla);
/// Aprende las reglas.
/// @param V las variables: los atributos y, al final, la clase.
/// @param E los ejemplos.
/// @param options ver inslv.hpp.
[[nodiscard]] std::vector<Regla> Aprender(const difuso::Variables& V, const datos::Ejemplos& E,
                                          const Opciones& options);

}  // namespace inslv
