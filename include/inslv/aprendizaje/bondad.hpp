// SPDX-License-Identifier: GPL-3.0-only
// La bondad de un individuo del genético y la búsqueda local que afina su regla.
#pragma once

#include <vector>

#include "inslv/aprendizaje/tabla.hpp"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/multipoblacion.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv::aprendizaje {

/// Lo que necesita la bondad de un individuo y no cambia durante un genético.
struct Evaluacion {
  genetico::MultiPoblacion& poblacion;
  const difuso::Variables& variables;
  const datos::Ejemplos& ejemplos;
  const reglas::ConjuntoDeReglas& reglas;
  /// Si solo se añaden reglas que eliminen a otras.
  bool reducir_reglas;
  /// Los ejemplos de cada clase sin clasificar bien.
  const std::vector<int>& sin_clasificar_por_clase;
  const TablaDeAdaptaciones& tabla;
  /// Por regla, los ejemplos que solo ella clasifica bien y los que clasifica mal por poco.
  const std::vector<int>& basica_para_regla;
  const std::vector<int>& errores_para_regla;
  /// Las primeras reglas no se pueden eliminar.
  int reglas_no_eliminables;
};

/// El peso del individuo y, en aciertos y fallos, lo que acierta y lo que falla.
double peso_del_individuo(const genetico::MultiPoblacion& poblacion,
                          const datos::Ejemplos& ejemplos, const TablaDeAdaptaciones& tabla,
                          int individuo, double& aciertos, double& fallos);
/// Valora el individuo y devuelve su peso. En reglas_a_eliminar, las reglas que puede sustituir (si
/// la regla no es válida, no lo cambia). Baja el nivel de los atributos que admiten todas sus
/// etiquetas.
double bondad(const Evaluacion& evaluacion, int individuo, genetico::Valoracion& valor,
              std::vector<int>& reglas_a_eliminar);
/// Valora los individuos modificados.
void valorar_modificados(const Evaluacion& evaluacion);
/// Generaliza la regla del individuo: quita atributos y admite etiquetas mientras no empeore.
void generalizar(const Evaluacion& evaluacion, int individuo, std::vector<int>& reglas_a_eliminar);
/// Especializa la regla del individuo: añade atributos y quita etiquetas mientras no empeore.
void especializar(const Evaluacion& evaluacion, int individuo, std::vector<int>& reglas_a_eliminar);

}  // namespace inslv::aprendizaje
