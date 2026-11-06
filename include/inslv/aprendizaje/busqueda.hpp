// SPDX-License-Identifier: GPL-3.0-only
// El algoritmo genético que busca cada regla.
#pragma once

#include <vector>

#include "inslv/aprendizaje/bondad.hpp"
#include "inslv/generador_de_azar.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv::aprendizaje {

/// Un genético entero: busca la siguiente regla y, si la hay, añade su versión descriptiva a
/// descriptivas, la discriminante a discriminantes y su patrón a patrones (en anadida, si la ha
/// añadido). En reglas_a_eliminar, las reglas que puede sustituir. medidas: las medidas de
/// información, para la población inicial; solo se crea una si primera_vez o si no hay reglas.
void buscar_regla(const Evaluacion& evaluacion, GeneradorDeAzar& azar,
                  reglas::ConjuntoDeReglas& descriptivas, reglas::ConjuntoDeReglas& discriminantes,
                  reglas::ConjuntoDeReglas& patrones,
                  const std::vector<std::vector<double>>& medidas, bool primera_vez,
                  std::vector<int>& reglas_a_eliminar, bool& anadida);

}  // namespace inslv::aprendizaje
