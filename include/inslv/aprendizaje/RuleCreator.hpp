// SPDX-License-Identifier: GPL-3.0-only
// El aprendizaje de las reglas: el algoritmo incremental, por episodios, y su genético.
#pragma once

#include <string>
#include <vector>

#include "inslv/aprendizaje/tabla.h"
#include "inslv/datos/example_set.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/genetico/genetcode.h"
#include "inslv/genetico/multipoblacion.h"
#include "inslv/inslv.hpp"
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
/// Un genético entero: busca la siguiente regla y, si la hay, la añade a R, R_dis y R_mix.
void GA(const Evaluacion& ev, ruleset& R, ruleset& R_dis, ruleset& R_mix,
        const std::vector<std::vector<double>>& I, bool primeraVez,
        std::vector<int>& reglas_a_eliminar, bool& Added_Rule);
/// Valora los individuos modificados.
void fitness(const Evaluacion& ev);
/// Generaliza la regla del individuo: quita atributos y admite etiquetas mientras no empeore.
void Obtener_Regla_Discriminante(const Evaluacion& ev, int elemento_i,
                                 std::vector<int>& reglas_a_eliminar);
/// Especializa la regla del individuo: añade atributos y quita etiquetas mientras no empeore.
void Obtener_Regla_Descriptiva(const Evaluacion& ev, int elemento_i,
                               std::vector<int>& reglas_a_eliminar);
/// Valora el individuo y devuelve su peso. En reglas_a_eliminar, las reglas que puede sustituir (si
/// la regla no es válida, no lo cambia).
double Bondad(const Evaluacion& ev, int elemento_i, Fitness& valor,
              std::vector<int>& reglas_a_eliminar);

/// Las etiquetas de la regla pos, o una cadena vacía si no existe.
std::string Patron1Regla(const ruleset& R_1, int pos);
/// El patrón de dos reglas emparejadas: sus etiquetas donde coinciden y '?' donde no.
std::string Patron2Reglas(const ruleset& R_1, const ruleset& R_2, const VectorVar& V, int pos);
/// La proporción de ejemplos que clasifican bien las reglas. Anota en E lo que da cada uno
/// (example_set::Mark).
double Aciertos(const ruleset& R, const VectorVar& V, example_set& E);
/// Las reglas que clasifican algún ejemplo, bien o mal.
std::vector<int> Reglas_Que_Disparan(const ruleset& R, const example_set& E);
void Filtrar_Reglas_New(ruleset& R, ruleset& R_dis, ruleset& R_mix,
                        const std::vector<int>& reglas_a_eliminar);
/// Una matriz con una fila por atributo y una columna por clase más una: la información mutua
/// normalizada entre el atributo y cada clase y, en la columna 0, con todas.
std::vector<std::vector<double>> MedidasDeInformacion(const VectorVar& V, const example_set& E);
/// Combina las nuevas (las primeras) con las demás reglas que tengan un patrón compatible.
void AnalizarConjuntosDeReglas(const VectorVar& V, ruleset& R, ruleset& R_Descrip, ruleset& R_mix,
                               int nuevas);
/// Actualiza con esos ejemplos el peso de las reglas modificadas. Usa el último individuo de G.
void ActualizarPeso(multipoblacion& G, const example_set& E, ruleset& R,
                    const TablaDeAdaptaciones& tabla);

/// Aprende las reglas con el algoritmo incremental, por episodios.
/// @param V las variables: los atributos y, al final, la clase.
/// @param E los ejemplos.
/// @param options ver inslv.hpp.
std::vector<Regla> Aprender(const VectorVar& V, const example_set& E, const Opciones& options);

}  // namespace inslv
