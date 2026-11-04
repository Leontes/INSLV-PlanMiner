// SPDX-License-Identifier: GPL-3.0-only
// El algoritmo incremental de INSLV: aprende las reglas por episodios.
#pragma once

#include <vector>

#include "inslv/aprendizaje/tabla.hpp"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/multipoblacion.hpp"
#include "inslv/inslv.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv::aprendizaje {

/// La proporción de ejemplos que clasifican bien las reglas. Anota en ejemplos lo que da cada uno
/// (datos::Ejemplos::Mark).
double aciertos(const reglas::ConjuntoDeReglas& reglas, const difuso::Variables& variables,
                datos::Ejemplos& ejemplos);
/// Las reglas que clasifican algún ejemplo, bien o mal.
[[nodiscard]] std::vector<int> reglas_que_disparan(const reglas::ConjuntoDeReglas& reglas,
                                                   const datos::Ejemplos& ejemplos);
/// Actualiza con esos ejemplos el peso de las reglas modificadas. Usa el último individuo de
/// poblacion.
void actualizar_pesos(genetico::MultiPoblacion& poblacion, const datos::Ejemplos& ejemplos,
                      reglas::ConjuntoDeReglas& reglas, const TablaDeAdaptaciones& tabla);
/// Aprende las reglas.
/// @param variables las variables: los atributos y, al final, la clase.
/// @param ejemplos los ejemplos.
/// @param opciones ver inslv.hpp.
[[nodiscard]] std::vector<Regla> aprender(const difuso::Variables& variables,
                                          const datos::Ejemplos& ejemplos,
                                          const Opciones& opciones);

}  // namespace inslv::aprendizaje
