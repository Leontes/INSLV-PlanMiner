// SPDX-License-Identifier: GPL-3.0-only
// El algoritmo incremental de INSLV: aprende las reglas por episodios.
#pragma once

#include <vector>

#include "inslv/aprendizaje/tabla.h"
#include "inslv/datos/example_set.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/genetico/multipoblacion.h"
#include "inslv/inslv.hpp"
#include "inslv/reglas/ruleset.h"

namespace inslv {

/// La proporción de ejemplos que clasifican bien las reglas. Anota en E lo que da cada uno
/// (example_set::Mark).
double Aciertos(const ruleset& R, const VectorVar& V, example_set& E);
/// Las reglas que clasifican algún ejemplo, bien o mal.
[[nodiscard]] std::vector<int> Reglas_Que_Disparan(const ruleset& R, const example_set& E);
/// Actualiza con esos ejemplos el peso de las reglas modificadas. Usa el último individuo de G.
void ActualizarPeso(multipoblacion& G, const example_set& E, ruleset& R,
                    const TablaDeAdaptaciones& tabla);
/// Aprende las reglas.
/// @param V las variables: los atributos y, al final, la clase.
/// @param E los ejemplos.
/// @param options ver inslv.hpp.
[[nodiscard]] std::vector<Regla> Aprender(const VectorVar& V, const example_set& E,
                                          const Opciones& options);

}  // namespace inslv
