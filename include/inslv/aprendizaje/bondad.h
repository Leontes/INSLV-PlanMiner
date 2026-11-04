// SPDX-License-Identifier: GPL-3.0-only
// La bondad de un individuo del genético y la búsqueda local que afina su regla.
#pragma once

#include <vector>

#include "inslv/aprendizaje/tabla.h"
#include "inslv/datos/example_set.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/genetico/multipoblacion.h"
#include "inslv/reglas/ruleset.h"

namespace inslv {

/// Lo que necesita la bondad de un individuo y no cambia durante un genético.
struct Evaluacion {
  multipoblacion& G;
  const VectorVar& V;
  const example_set& E;
  const ruleset& R;
  /// Si solo se añaden reglas que eliminen a otras.
  bool reducir_reglas;
  /// Los ejemplos de cada clase sin clasificar bien.
  const std::vector<int>& ponderacion_por_clase;
  const TablaDeAdaptaciones& tabla;
  /// Por regla, los ejemplos que solo ella clasifica bien y los que clasifica mal por poco.
  const std::vector<int>& basica_para_regla;
  const std::vector<int>& errores_para_regla;
  /// Las primeras reglas no se pueden eliminar.
  int tam_rule_not_removed;
};

/// El peso del individuo, y en n_mas y n_menos, los ejemplos que acierta y los que falla.
double Weight_Individual(const multipoblacion& G, const example_set& E,
                         const TablaDeAdaptaciones& tabla, int elemento_i, double& n_mas,
                         double& n_menos);
/// Valora el individuo y devuelve su peso. En reglas_a_eliminar, las reglas que puede sustituir (si
/// la regla no es válida, no lo cambia). Baja el nivel de los atributos que admiten todas sus
/// etiquetas.
double Bondad(const Evaluacion& ev, int elemento_i, Fitness& valor,
              std::vector<int>& reglas_a_eliminar);
/// Valora los individuos modificados.
void fitness(const Evaluacion& ev);
/// Generaliza la regla del individuo: quita atributos y admite etiquetas mientras no empeore.
void Obtener_Regla_Discriminante(const Evaluacion& ev, int elemento_i,
                                 std::vector<int>& reglas_a_eliminar);
/// Especializa la regla del individuo: añade atributos y quita etiquetas mientras no empeore.
void Obtener_Regla_Descriptiva(const Evaluacion& ev, int elemento_i,
                               std::vector<int>& reglas_a_eliminar);

}  // namespace inslv
