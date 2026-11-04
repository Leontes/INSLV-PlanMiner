// SPDX-License-Identifier: GPL-3.0-only
#ifndef RULECREATOR_HPP
#define RULECREATOR_HPP

#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <math.h>
#include <fstream>
#include <vector>
#include <queue>
#include "inslv/difuso/domain_t.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/datos/vectordouble.h"
#include "inslv/datos/example_set.h"
#include "inslv/genetico/multipoblacion.h"
#include "inslv/genetico/genetcode.h"
#include "inslv/reglas/ruleset.h"
#include "inslv/genetico/operaciones.h"

namespace inslv {

// Código heredado: usa string, cout... sin std:: (hasta que se modernice aprendizaje).
using namespace std;


//Funciones de SLAVE...
double Weight_Individual(multipoblacion &G, const VectorVar &V, const example_set &E, VectorVar &F, double ***Tabla, double ***Tabla_Nuevas, int elemento_i,
			 double &n_mas, double &n_menos);
int GA(multipoblacion &G, const VectorVar &V, example_set &E, ruleset &R, ruleset &R_dis, ruleset &R_mix, double **I, double *agregado,
       double *peso_agregado, int Total_ejemplos, bool discriminante, bool primeraVez, bool con_peso,
       int *ponderacion_por_clase, double ***Tabla, double ***Tabla_Nuevas, VectorVar &F, int *basica_para_regla,
       int *errores_para_regla, int *reglas_a_eliminar, int &n_reglas_a_eliminar, bool &Added_Rule, bool &Specific_Rule,
       int tam_rule_not_removed);
void fitness (multipoblacion &G, const VectorVar &V, const example_set &E, ruleset &R, bool discriminante, bool con_peso,
              int *ponderacion_por_clase, int nrule, double ***Tabla, double ***Tabla_Nuevas, VectorVar &F, int *basica_para_regla,
	      int *errores_para_regla, int tam_rule_not_removed);
void Obtener_Regla_Discriminante(multipoblacion &G, const VectorVar &V, example_set &E, ruleset &R, int elemento_i,
                               bool discriminante, bool reducir_reglas, int* ponderacion_por_clase, double ***Tabla,
                               double ***Tabla_Nuevas, VectorVar &F, int *basica_para_regla,
			       int *errores_para_regla, int *reglas_a_eliminar, int &n_reglas_a_eliminar, bool debug,
			       string patr, bool inGA, string initial_rule, int tam_rule_not_removed);
string Patron1Regla(ruleset &R_1, const VectorVar &V, int pos);
string Patron2Patrones(string pat1, string pat2);
bool ComprobacionEspGen(string esp, string gen);
string Patron2Reglas(ruleset &R_1, ruleset &R_2, const VectorVar &V, int pos);
void Obtener_Regla_Descriptiva(multipoblacion &G, const VectorVar &V, example_set &E, ruleset &R, int elemento_i,
                               bool discriminante, bool reducir_reglas, int* ponderacion_por_clase, double ***Tabla,
                               double ***Tabla_Nuevas, VectorVar &F, int *basica_para_regla,
			       int *errores_para_regla, int *reglas_a_eliminar, int &n_reglas_a_eliminar, bool debug,
			       string patr, bool inGA, string initial_rule, int tam_rule_not_removed);
double Bondad (multipoblacion &G, const VectorVar &V, const example_set &E, ruleset &R, double *valor, int N_valor,
               int elemento_i, bool discriminante, bool con_peso, int *ponderacion_por_clase, int nrule,
               double ***Tabla, double ***Tabla_Nuevas, VectorVar &F, int *basica_para_regla, int *errores_para_regla, int *reglas_a_eliminar, int &n_reglas_a_eliminar,
	       bool esp_gen, string initial_rule, int tam_rule_not_removed);
double Aciertos(ruleset &R, const VectorVar &V, example_set &E, VectorVar &F);
double Aciertos(ruleset &R, const VectorVar &V, example_set &E, string nomfich, VectorVar &F);
double Filtrar_Reglas_New(ruleset &R, ruleset &R_dis, ruleset &R_mix, int *reglas_a_eliminar,
			  int n_reglas_a_eliminar, bool Specific_Rule);
double Inferencia2BasesReglas(ruleset &R, ruleset &R_Descrip, const VectorVar &V, example_set &E, int *fallosR, int *fallosR_Descrip, int *disparosR, int *disparosR_Descrip);
double **ReservaParaMedidasDeInformacion(const VectorVar &V);
void LiberaMedidasDeInformacion(double **&matriz, int n);
void MedidasDeInformacion(const VectorVar &V, const example_set &E, double **I);
void Obtener_Aciertos_y_Errores_Reglas_Aprendidas(example_set &E, int *basica_para_regla, int *errores_para_regla, int nrule);
double Aciertos_Lista_Reglas_Eliminables(ruleset &R, const VectorVar &V, example_set &E, VectorVar &F, int*lista, int &tama, int *l_aciertos, int &tama_l);

//---- Funciones para manejar la tabla con las adaptaciones de los ejemplos a las etiquetas de las variables
double ***Crear_Tabla_Adaptaciones(const VectorVar &V, const example_set &E);
double ***Crear_Tabla_Adaptaciones_Nuevas(const VectorVar &V, const example_set &E);
void Destruir_Tabla_Adaptaciones(const VectorVar &V, const example_set &E, double ***&Tabla);
double T_Adaptacion_Variable_Etiqueta(const VectorVar &V, double ***Tabla, int ejemplo,
                                      int variable, int etiqueta);
double T_Adaptacion_Variable(const VectorVar &V, double ***Tabla, int ejemplo, int variable, string labels);
double T_Adaptacion_Antecedente(const VectorVar &V, double ***Tabla, int ejemplo, string regla,
                                double *nivel_variable, double umbral);
double T_Adaptacion_Antecedente_Nuevas(const VectorVar &V, double ***Tabla, int ejemplo, string regla,
                                       int *nivel_variable_funciones, int tama_nivel_variable_funciones, double matching_previo);
void T_AdaptacionC(const VectorVar &V, double ***Tabla, int ejemplo, int etiq, double &pos, double &neg);
int BetterEtiqueta(const VectorVar &V,double ***Tabla, int variable, int ejemplo);
int Comparar2patrones(const string &cad1, const string &cad2);
int Comparar2patronesPinta(const string &cad1, const string &cad2);
bool AnalizarConjuntosDeReglas(const VectorVar &V, ruleset &R, ruleset &R_Descrip, ruleset &R_mix, int nuevas);
string Interseccion(const string &cad1, const string &cad2);
string Union(const string &cad1, const string &cad2);
int Grados_Libertad(string cad);
int DistanciaHamming(const string &s1, const string &s2);
void ActualizarPeso(multipoblacion &G, const VectorVar &V, example_set &E, ruleset &R, VectorVar &F,
       double ***Tabla, double ***Tabla_Nuevas);
bool vectoresIguales(double *v1, double *v2, int tama);



/** Aprender
*   @brief Aprende las reglas descriptivas con el algoritmo incremental (por episodios).
*   @param V las variables: los atributos y, al final, la clase.
*   @param E el conjunto de ejemplos.
*   @param options semilla e informe (ver inslv.hpp).
*   @retval las reglas descriptivas.
*/
std::vector<Regla> Aprender(VectorVar &V, example_set &E, const Opciones &options);


}  // namespace inslv

#endif
