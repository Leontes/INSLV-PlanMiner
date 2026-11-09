// SPDX-License-Identifier: GPL-3.0-only
// Las reglas características: las descriptivas con todo lo que cumplen sus ejemplos.
#pragma once

#include <vector>

#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/inslv.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv::aprendizaje {

/// Las condiciones de la regla y, además, las que cumple al menos 1 - tolerancia de los ejemplos de
/// su clase que cubre: para cada atributo, el menor conjunto de etiquetas (dentro de las que admite
/// la regla) a las que se adaptan esos ejemplos, empezando por las de más ejemplos. Si son todas
/// las del atributo, no hay condición. Si la regla no cubre ningún ejemplo de su clase, sus
/// condiciones.
///
/// Las descriptivas solo tienen las condiciones que no cuestan bondad: con ruido, casi ninguna se
/// cumple en todos los ejemplos, y se quedan fuera aunque las cumplan casi todos.
[[nodiscard]] std::vector<Condicion> caracterizar(const reglas::ConjuntoDeReglas& reglas, int regla,
                                                  const difuso::Variables& variables,
                                                  const datos::Ejemplos& ejemplos,
                                                  double tolerancia);

}  // namespace inslv::aprendizaje
