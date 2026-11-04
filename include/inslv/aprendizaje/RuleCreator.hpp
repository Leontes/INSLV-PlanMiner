// SPDX-License-Identifier: GPL-3.0-only
#ifndef RULECREATOR_HPP
#define RULECREATOR_HPP

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>

#include <array>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "inslv/datos/example_set.h"
#include "inslv/datos/vectordouble.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/difuso/domain_t.h"
#include "inslv/genetico/genetcode.h"
#include "inslv/genetico/multipoblacion.h"
#include "inslv/genetico/operaciones.h"
#include "inslv/reglas/ruleset.h"

namespace inslv {

// Código heredado: usa string, cout... sin std:: (hasta que se modernice aprendizaje).
using namespace std;

/// La valoración de un individuo: sus valores se comparan en orden.
using Fitness = std::array<double, 7>;

/// Lo que necesita la bondad de un individuo y no cambia durante un genético.
struct Evaluacion {
  multipoblacion& G;
  const VectorVar& V;
  const example_set& E;
  const ruleset& R;
  /// Si solo se añaden reglas que eliminen a otras.
  bool reducir_reglas;
  /// Los ejemplos de cada clase sin clasificar bien.
  const int* ponderacion_por_clase;
  double*** Tabla;
  /// Por regla, los ejemplos que solo ella clasifica bien, y los que clasifica mal por poco.
  const int* basica_para_regla;
  const int* errores_para_regla;
  /// Las primeras reglas no se pueden eliminar.
  int tam_rule_not_removed;
};

double Weight_Individual(multipoblacion& G, const VectorVar& V, const example_set& E,
                         double*** Tabla, int elemento_i, double& n_mas, double& n_menos);
void GA(const Evaluacion& ev, ruleset& R, ruleset& R_dis, ruleset& R_mix, double** I,
        bool primeraVez, std::vector<int>& reglas_a_eliminar, bool& Added_Rule);
void fitness(const Evaluacion& ev);
void Obtener_Regla_Discriminante(const Evaluacion& ev, int elemento_i,
                                 std::vector<int>& reglas_a_eliminar);
void Obtener_Regla_Descriptiva(const Evaluacion& ev, int elemento_i,
                               std::vector<int>& reglas_a_eliminar);
/// Valora el individuo y devuelve su peso. En reglas_a_eliminar, las reglas que puede sustituir (si
/// la regla no es válida, no lo cambia).
double Bondad(const Evaluacion& ev, int elemento_i, Fitness& valor,
              std::vector<int>& reglas_a_eliminar);
string Patron1Regla(ruleset& R_1, const VectorVar& V, int pos);
string Patron2Patrones(string pat1, string pat2);
bool ComprobacionEspGen(string esp, string gen);
string Patron2Reglas(ruleset& R_1, ruleset& R_2, const VectorVar& V, int pos);
/// La proporción de ejemplos que clasifican bien las reglas. Anota en E lo que da cada uno
/// (example_set::Mark).
double Aciertos(const ruleset& R, const VectorVar& V, example_set& E);
/// Las reglas que clasifican algún ejemplo, bien o mal.
std::vector<int> Reglas_Que_Disparan(const ruleset& R, const example_set& E);
void Filtrar_Reglas_New(ruleset& R, ruleset& R_dis, ruleset& R_mix,
                        const std::vector<int>& reglas_a_eliminar);
double** ReservaParaMedidasDeInformacion(const VectorVar& V);
void LiberaMedidasDeInformacion(double**& matriz, int n);
void MedidasDeInformacion(const VectorVar& V, const example_set& E, double** I);
void Obtener_Aciertos_y_Errores_Reglas_Aprendidas(const example_set& E, int* basica_para_regla,
                                                  int* errores_para_regla, int nrule);

// La tabla con las adaptaciones de los ejemplos a las etiquetas de las variables.
double*** Crear_Tabla_Adaptaciones(const VectorVar& V, const example_set& E);
void Destruir_Tabla_Adaptaciones(const VectorVar& V, double***& Tabla);
double T_Adaptacion_Variable_Etiqueta(const VectorVar& V, double*** Tabla, int ejemplo,
                                      int variable, int etiqueta);
double T_Adaptacion_Variable(const VectorVar& V, double*** Tabla, int ejemplo, int variable,
                             string labels);
double T_Adaptacion_Antecedente(const VectorVar& V, double*** Tabla, int ejemplo,
                                const string& regla, const double* nivel_variable, double umbral);
void T_AdaptacionC(const VectorVar& V, double*** Tabla, int ejemplo, int etiq, double& pos,
                   double& neg);
int BetterEtiqueta(const VectorVar& V, double*** Tabla, int variable, int ejemplo);
int Comparar2patrones(const string& cad1, const string& cad2);
int Comparar2patronesPinta(const string& cad1, const string& cad2);
void AnalizarConjuntosDeReglas(const VectorVar& V, ruleset& R, ruleset& R_Descrip, ruleset& R_mix,
                               int nuevas);
string Interseccion(const string& cad1, const string& cad2);
string Union(const string& cad1, const string& cad2);
int Grados_Libertad(string cad);
void ActualizarPeso(multipoblacion& G, const VectorVar& V, example_set& E, ruleset& R,
                    double*** Tabla);

/// Aprende las reglas con el algoritmo incremental, por episodios.
/// @param V las variables: los atributos y, al final, la clase.
/// @param E los ejemplos.
/// @param options ver inslv.hpp.
std::vector<Regla> Aprender(const VectorVar& V, const example_set& E, const Opciones& options);

}  // namespace inslv

#endif
