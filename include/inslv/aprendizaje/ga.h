// SPDX-License-Identifier: GPL-3.0-only
// El algoritmo genético que busca cada regla.
#pragma once

#include <vector>

#include "inslv/aprendizaje/bondad.h"
#include "inslv/reglas/ruleset.h"

namespace inslv {

/// Un genético entero: busca la siguiente regla y, si la hay, añade su versión descriptiva a R, la
/// discriminante a R_dis y su patrón a R_mix (en Added_Rule, si la ha añadido). En
/// reglas_a_eliminar, las reglas que puede sustituir. I: las medidas de información, para la
/// población inicial; solo se crea una si primeraVez o si no hay reglas.
void GA(const Evaluacion& ev, ruleset& R, ruleset& R_dis, ruleset& R_mix,
        const std::vector<std::vector<double>>& I, bool primeraVez,
        std::vector<int>& reglas_a_eliminar, bool& Added_Rule);

}  // namespace inslv
