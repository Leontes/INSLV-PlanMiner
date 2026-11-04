// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/RuleCreator.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <format>
#include <fstream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Si la valoración v2 es mejor que la v en sus tres primeros valores.
bool Mejor(const Fitness& v2, const Fitness& v) {
  return (v2[0] > v[0]) || (v2[0] == v[0] && v2[1] > v[1]) ||
         (v2[0] == v[0] && v2[1] == v[1] && v2[2] > v[2]);
}

// Igual, o si es igual de buena.
bool MejorOIgual(const Fitness& v2, const Fitness& v) {
  return (v2[0] > v[0]) || (v2[0] == v[0] && v2[1] > v[1]) ||
         (v2[0] == v[0] && v2[1] == v[1] && v2[2] >= v[2]);
}

// En el original esperaba una tecla tras los avisos de consistencia del análisis de patrones.
// Como librería no puede bloquearse esperando la entrada estándar: solo avisa.
void Pausa() {
  progreso(Verbosidad::detalle)
      << "INSLV: aviso de consistencia en el análisis de patrones (ver la salida anterior)\n";
}

// 0 si los patrones son iguales; 1 si son compatibles (solo difieren en algún '?'); 2 si no.
int Comparar2patrones(const std::string& cad1, const std::string& cad2) {
  int n_cambios = 0;
  bool compatibles = true;
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    if (cad1[s] != cad2[s]) {
      if ((cad1[s] == '0' && cad2[s] == '1') || (cad1[s] == '1' && cad2[s] == '0')) {
        compatibles = false;
      }
      ++n_cambios;
    }
  }
  if (n_cambios == 0) {
    return 0;
  }
  return compatibles ? 1 : 2;
}

// Las diferencias entre dos patrones, a la depuración.
void Comparar2patronesPinta(const std::string& cad1, const std::string& cad2) {
  std::ostream& salida = progreso(Verbosidad::depuracion);
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    if (cad1[s] == cad2[s]) {
      salida << '_';
    } else if (cad1[s] == '0' && cad2[s] == '1') {
      salida << '>';
    } else if (cad1[s] == '1' && cad2[s] == '0') {
      salida << '<';
    } else {
      salida << '$';
    }
  }
  salida << '\n';
}

// Las etiquetas de la regla, con los atributos por debajo del umbral admitiendo todas.
std::string EtiquetasEfectivas(const genetcode& code, const VectorVar& V) {
  std::string etiquetas = code.etiquetas;
  std::size_t tama = 0;
  for (int j = 0; j < V.N_Antecedente(); ++j) {
    const auto longitud = indice(V.SizeDomain(j));
    if (code.niveles[indice(j)] < code.niveles.back()) {
      etiquetas.replace(tama, longitud, longitud, '1');
    }
    tama += longitud;
  }
  return etiquetas;
}

std::string Union(const std::string& cad1, const std::string& cad2) {
  std::string aux = cad1;
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    aux[s] = cad1[s] == '0' && cad2[s] == '0' ? '0' : '1';
  }
  return aux;
}

std::string Interseccion(const std::string& cad1, const std::string& cad2) {
  std::string aux = cad1;
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    aux[s] = cad1[s] == '1' && cad2[s] == '1' ? '1' : '0';
  }
  return aux;
}

// Hace la union de las dos reglas y la operacion la almacena en la regla r1
void UnionReglasDescriptivas(const VectorVar& V, ruleset& R1, int r1, const ruleset& R2, int r2) {
  const std::string patron = Union(EtiquetasEfectivas(R1.Get_Code_Rule(r1), V),
                                   EtiquetasEfectivas(R2.Get_Code_Rule(r2), V));
  R1.Get_Code_Rule(r1).etiquetas = patron;
}

// Hace la interseccion de las dos reglas y la operacion la almacena en la regla r1
void IntersecionReglasDiscriminantes(const VectorVar& V, ruleset& R1, int r1, const ruleset& R2,
                                     int r2) {
  const std::string patron = Interseccion(EtiquetasEfectivas(R1.Get_Code_Rule(r1), V),
                                          EtiquetasEfectivas(R2.Get_Code_Rule(r2), V));
  genetcode& code = R1.Get_Code_Rule(r1);
  code.etiquetas = patron;
  for (int j = 0; j < V.N_Antecedente(); ++j) {
    if (code.niveles[indice(j)] < code.niveles.back()) {
      code.niveles[indice(j)] = code.niveles.back();
    }
  }
}

int Grados_Libertad(const std::string& cad) {
  return static_cast<int>(std::count(cad.begin(), cad.end(), '?'));
}

// Determina si se mantiene que esp es una especificacion de gen.
bool ComprobacionEspGen(const std::string& esp, const std::string& gen) {
  for (std::size_t i = 0; i < esp.size(); ++i) {
    if (esp[i] == '1' && gen[i] == '0') {
      return false;
    }
  }
  return true;
}

std::string Patron2Patrones(const std::string& pat1, const std::string& pat2) {
  std::string pat3 = pat1;
  for (std::size_t i = 0; i < pat1.size(); ++i) {
    if (pat1[i] == pat2[i]) {
      pat3[i] = pat1[i];
    } else if ((pat1[i] == '1' && pat2[i] == '?') || (pat1[i] == '?' && pat2[i] == '1')) {
      pat3[i] = '1';
    } else if ((pat1[i] == '0' && pat2[i] == '?') || (pat1[i] == '?' && pat2[i] == '0')) {
      pat3[i] = '0';
    } else {
      pat3[i] = '?';
    }
  }
  return pat3;
}

// Un individuo por cada elemento de la población: un ejemplo al azar de su clase sin clasificar
// bien, como la mejor etiqueta de cada atributo; -1 si no quedan ejemplos de la clase.
std::vector<std::vector<int>> Encontrar_Individuos_dados_ejemplos(
    const VectorVar& V, const example_set& E, int nclases, int n_individuos,
    const TablaDeAdaptaciones& tabla, const std::vector<int>& n_ejemplos_por_clase) {
  std::vector<std::vector<int>> matriz(indice(n_individuos),
                                       std::vector<int>(indice(V.N_Antecedente()), -1));
  for (int i = 0; i < n_individuos; ++i) {
    if (n_ejemplos_por_clase[indice(i % nclases)] != 0) {
      int a = Select_Random_Individuo(E.N_Examples(), -1, 0);
      while (E.Is_Covered(a) || tabla.Etiqueta(a, V.Consecuente(), i % nclases) == 0) {
        a = (a + 1) % E.N_Examples();
      }
      for (int j = 0; j < V.N_Antecedente(); ++j) {
        matriz[indice(i)][indice(j)] = tabla.MejorEtiqueta(a, j);
      }
    }
  }
  return matriz;
}

// Si s tiene algún cero: si no, el atributo admite todas sus etiquetas y es irrelevante.
bool Relevante(std::string_view s) {
  return !std::all_of(s.begin(), s.end(), [](char bit) { return bit == '1'; });
}

// Por regla, los ejemplos que solo ella clasifica bien y los que clasifica mal por poco.
void Obtener_Aciertos_y_Errores_Reglas_Aprendidas(const example_set& E,
                                                  std::vector<int>& basica_para_regla,
                                                  std::vector<int>& errores_para_regla) {
  for (int j = 0; j < E.N_Examples(); ++j) {
    if (E.Rule_Aislada(j) == 1 && E.Rule_Used(j) >= 0) {
      ++basica_para_regla[indice(E.Rule_Used(j))];
    } else if (E.Rule_Aislada(j) == -1 && E.Rule_Used(j) >= 0) {
      ++errores_para_regla[indice(E.Rule_Used(j))];
    }
  }
}

// Las probabilidades de cada etiqueta de la variable en los ejemplos sin clasificar bien.
std::vector<double> CalcularProbabilidadSimpleVariable(const VectorVar& V, const example_set& E,
                                                       int variable) {
  const auto n_casos = indice(V.SizeDomain(variable));
  std::vector<double> p(n_casos, 0);
  std::vector<double> aux(n_casos);
  int ne = 0;
  for (int i = 0; i < E.N_Examples(); ++i) {
    if (!E.Is_Covered(i)) {
      double suma = 0;
      for (std::size_t j = 0; j < n_casos; ++j) {
        aux[j] = V.Domain(variable).Adaptacion(E.Data(i, variable), static_cast<int>(j));
        suma += aux[j];
      }
      // Añadir los valores a la sumatoria
      if (suma > 0) {
        for (std::size_t j = 0; j < n_casos; ++j) {
          p[j] = p[j] + (aux[j] / suma);
        }
        ++ne;
      }
    }
  }
  if (ne > 0) {
    progreso(Verbosidad::depuracion) << "NE: " << ne << '\n';
    for (double& probabilidad : p) {
      probabilidad = probabilidad / ne;
    }
  }
  return p;
}

// Las probabilidades de cada par de etiquetas de dos variables, igual.
std::vector<std::vector<double>> CalcularProbabilidadConjunta2Variables(const VectorVar& V,
                                                                        const example_set& E,
                                                                        int var1, int var2) {
  const auto n_casos1 = indice(V.SizeDomain(var1));
  const auto n_casos2 = indice(V.SizeDomain(var2));
  std::vector<std::vector<double>> m(n_casos1, std::vector<double>(n_casos2, 0));
  std::vector<std::vector<double>> aux(n_casos1, std::vector<double>(n_casos2));
  int ne = 0;
  for (int i = 0; i < E.N_Examples(); ++i) {
    if (!E.Is_Covered(i)) {
      double suma = 0;
      for (std::size_t j = 0; j < n_casos1; ++j) {
        for (std::size_t k = 0; k < n_casos2; ++k) {
          aux[j][k] = V.Domain(var1).Adaptacion(E.Data(i, var1), static_cast<int>(j)) *
                      V.Domain(var2).Adaptacion(E.Data(i, var2), static_cast<int>(k));
          suma = suma + aux[j][k];
        }
      }
      if (suma > 0) {
        for (std::size_t j = 0; j < n_casos1; ++j) {
          for (std::size_t k = 0; k < n_casos2; ++k) {
            m[j][k] = m[j][k] + (aux[j][k] / suma);
          }
        }
      }
      ++ne;
    }
  }
  if (ne > 0) {
    for (std::vector<double>& fila : m) {
      for (double& probabilidad : fila) {
        probabilidad = probabilidad / ne;
      }
    }
  }
  return m;
}

// Los aciertos de cada regla, para el informe (marca los ejemplos, como Aciertos).
double Aciertos_Por_Regla(const ruleset& R, const VectorVar& V, example_set& E,
                          const std::string& nomfich) {
  const double acierto = Aciertos(R, V, E);
  std::vector<int> bien_clas(indice(R.N_rule()));
  std::vector<int> mal_clas(indice(R.N_rule()));
  int bien = 0;
  int mal = 0;
  int nosesabe = 0;
  for (int i = 0; i < E.N_Examples(); ++i) {
    if (E.Rule_Used(i) == -1) {
      ++nosesabe;
    } else if (E.Is_Covered(i)) {
      ++bien;
      ++bien_clas[indice(E.Rule_Used(i))];
    } else {
      ++mal;
      ++mal_clas[indice(E.Rule_Used(i))];
    }
  }

  std::ofstream out(nomfich, std::ios::app);
  out << "Aciertos: " << bien << '\n';
  out << "Errores:  " << mal << '\n';
  out << "No clasificados: " << nosesabe << '\n';
  for (int i = 0; i < R.N_rule(); ++i) {
    out << "\tRule " << i << "(" << R.SPrint_Consequent(i) << "): " << bien_clas[indice(i)] << " / "
        << mal_clas[indice(i)] << '\n';
  }
  return acierto;
}

void Escribir_Informe(const std::string& task_res, const VectorVar& V, example_set& E,
                      const ruleset& R_prev, const ruleset& R_prev_dis, const ruleset& R_mix_prev,
                      double tiempo) {
  const double aciertos_discriminantes = Aciertos_Por_Regla(R_prev_dis, V, E, task_res);
  const double aciertos_descriptivas = Aciertos_Por_Regla(R_prev, V, E, task_res);

  std::vector<int> frecuencia_variables;
  const double variables_usadas = R_prev.Frecuence_each_Variables(frecuencia_variables);

  std::ofstream out(task_res, std::ios::app);
  out << "----------------------------------------------------\n";
  out << "Numero de reglas: " << R_prev.N_rule() << '\n';
  out << "Variables por regla: " << R_prev.Variables_per_rule() << '\n';
  out << "Variables usadas: " << variables_usadas << '\n';
  out << "Tiempo: " << tiempo << '\n';
  out << "Condiciones: " << R_prev.Condiciones_per_RB() << '\n';
  out << "Aciertos Discriminantes: " << aciertos_discriminantes << '\n';
  out << "Aciertos Descriptivas: " << aciertos_descriptivas << '\n';
  out << "----------------------------------------------------\n";
  out << "------------------- REGLAS -------------------------\n";
  out << "----------------------------------------------------\n";

  out << "Descriptivas: \n";
  out.close();
  R_prev.SaveRuleInterpreted_append(task_res);
  out.open(task_res, std::ios::app);
  out << "Predictivas: \n";
  out.close();
  R_prev_dis.SaveRuleInterpreted_append(task_res);
  out.open(task_res, std::ios::app);
  out << "Patrones: \n";

  for (int kk = 0; kk < R_mix_prev.N_rule(); ++kk) {
    out << Patron1Regla(R_mix_prev, kk) << '(' << R_mix_prev.Class_of_rule(kk) << ")\n";
  }

  out << "Frecuencia de aparicion de las " << V.N_Antecedente()
      << " distintas variables en las reglas: \n";
  for (int i = 0; i < V.N_Antecedente(); ++i) {
    out << V.SPrintVar(i) << ": --> "
        << (1.0 * frecuencia_variables[indice(i)] / R_prev.N_rule()) * 100 << " %\n";
  }
}

}  // namespace

void AnalizarConjuntosDeReglas(const VectorVar& V, ruleset& R, ruleset& R_Descrip, ruleset& R_mix,
                               int nuevas) {
  std::vector<bool> eliminar(indice(R.N_rule()), false);

  for (int i = 0; i < nuevas; ++i) {
    if (eliminar[indice(i)]) {
      continue;
    }
    const std::string res = Patron2Reglas(R_Descrip, R, V, i);
    const std::string res3 = Patron1Regla(R_Descrip, i);
    const std::string res4 = Patron1Regla(R, i);
    const std::string patr = R_mix.Get_Code_Rule(i).etiquetas;

    if (res != patr) {
      progreso(Verbosidad::depuracion) << "No coinciden los patrones para la regla " << i << '\n'
                                       << res << '\n'
                                       << patr << '\n';
      Pausa();
    }

    if (res4 != patr && Comparar2patrones(patr, res4) != 1) {
      progreso(Verbosidad::depuracion)
          << "La regla discriminante " << i << " no coincide con su patron\n"
          << res4 << '\n'
          << patr << '\n';
      Pausa();
    }

    if (res3 != patr && Comparar2patrones(patr, res3) != 1) {
      progreso(Verbosidad::depuracion)
          << "La regla descriptiva " << i << " no coincide con su patron\n"
          << res3 << '\n'
          << patr << '\n';
      Pausa();
    }

    for (int j = i + 1; j < R.N_rule() && !eliminar[indice(i)]; ++j) {
      if (eliminar[indice(j)]) {
        continue;
      }
      const std::string res2 = Patron2Reglas(R_Descrip, R, V, j);
      const int iguales = Comparar2patrones(res2, patr);

      if (R.Class_of_rule(i) != R.Class_of_rule(j)) {
        if (iguales == 0) {
          progreso(Verbosidad::detalle)
              << "las reglas " << i << " y " << j << " son contradictorias\n";
          R.Add_N_Menos(i, R.Get_N_Mas(i));
          R.Recalculate_Weight(i);
          R_Descrip.Add_N_Menos(i, R_Descrip.Get_N_Mas(i));
          R_Descrip.Recalculate_Weight(i);
        } else if (iguales == 1) {
          progreso(Verbosidad::detalle)
              << "las reglas " << i << " y " << j << " entran en conflicto\n";
        }
        continue;
      }

      // El caso raro de compatibilidades entre patrones
      if (iguales > 1 && Comparar2patrones(res3, res2) != 2 && Comparar2patrones(res4, res2) != 2) {
        progreso(Verbosidad::depuracion)
            << "Caso especial de incompatibilidades entre patrones y si entre reglas\n"
            << res3 << '\n'
            << res4 << '\n'
            << patr << "\n\n"
            << Patron1Regla(R_Descrip, j) << '\n'
            << Patron1Regla(R, j) << '\n'
            << res2 << "\n\n";
        Pausa();
      }

      if (iguales == 0) {
        // Los dos patrones son iguales
        eliminar[indice(i)] = true;
      } else if (iguales == 1) {
        // Los dos patrones son compatibles
        progreso(Verbosidad::detalle) << "\t\t\t\t\tCombinando Reglas " << i << " y " << j
                                      << " de la clase " << R.Class_of_rule(i) << '\n';

        const std::string cad1 = Patron2Reglas(R_Descrip, R, V, i);
        const std::string cad3 = Patron2Reglas(R_Descrip, R, V, j);

        IntersecionReglasDiscriminantes(V, R, j, R, i);
        UnionReglasDescriptivas(V, R_Descrip, j, R_Descrip, i);

        const std::string cad2 = Patron2Reglas(R_Descrip, R, V, j);
        const std::string cad4 = Patron2Patrones(cad1, cad3);
        progreso(Verbosidad::depuracion) << cad4 << '\n';
        if (Comparar2patrones(cad2, cad4) != 0) {
          progreso(Verbosidad::depuracion) << cad2 << '\n' << cad4 << '\n';
          Comparar2patronesPinta(cad2, cad4);
          Pausa();
        }

        R_mix.Get_Code_Rule(j).etiquetas = cad4;
        std::string discriminante = cad4;
        std::replace(discriminante.begin(), discriminante.end(), '?', '1');
        R.Get_Code_Rule(j).etiquetas = discriminante;
        std::string descriptiva = cad4;
        std::replace(descriptiva.begin(), descriptiva.end(), '?', '0');
        R_Descrip.Get_Code_Rule(j).etiquetas = descriptiva;

        if (Patron2Reglas(R_Descrip, R, V, j) != cad4) {
          progreso(Verbosidad::depuracion)
              << "Error al componer la descrip. y la predic. en la regla " << j << '\n';
          Pausa();
        }

        eliminar[indice(i)] = true;
        R_Descrip.Modificada(j, true);
        R.Modificada(j, true);

        if (Grados_Libertad(res2) >= Grados_Libertad(patr)) {
          R_Descrip.Add_N_Mas(j, R_Descrip.Get_N_Mas(i));
          R_Descrip.Add_N_Menos(j, R_Descrip.Get_N_Menos(i));
          R.Add_N_Mas(j, R.Get_N_Mas(i));
          R.Add_N_Menos(j, R.Get_N_Menos(i));
        }
      }
    }
  }

  std::vector<int> eliminables;
  for (int i = 0; i < R.N_rule() - 1; ++i) {
    if (eliminar[indice(i)]) {
      eliminables.push_back(i);
    }
  }

  Filtrar_Reglas_New(R, R_Descrip, R_mix, eliminables);
}

void ActualizarPeso(multipoblacion& G, const example_set& E, ruleset& R,
                    const TablaDeAdaptaciones& tabla) {
  double n_mas = 0;
  double n_menos = 0;
  for (int i = 0; i < R.N_rule(); ++i) {
    if (R.Get_Modificada(i)) {
      G.PutCodigo(G.N_individuos() - 1, R.Get_Code_Rule(i));
      Weight_Individual(G, E, tabla, G.N_individuos() - 1, n_mas, n_menos);
      R.Add_Weight(i, n_mas, n_menos);
      R.Modificada(i, false);
    }
  }
}

void GA(const Evaluacion& ev, ruleset& R, ruleset& R_dis, ruleset& R_mix,
        const std::vector<std::vector<double>>& I, bool primeraVez,
        std::vector<int>& reglas_a_eliminar, bool& Added_Rule) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  const example_set& E = ev.E;

  Added_Rule = false;

  const int n_clases = V.SizeDomain(V.Consecuente());

  if (R.N_rule() == 0 || primeraVez) {
    const std::vector<int> n_ejemplos_por_clase = E.Examples_per_Class(V.Consecuente(), n_clases);
    const std::vector<std::vector<int>> sujetos = Encontrar_Individuos_dados_ejemplos(
        V, E, n_clases, G.N_individuos(), ev.tabla, n_ejemplos_por_clase);
    std::vector<int> lista_tamano;
    for (int i = 0; i < V.N_Antecedente(); ++i) {
      lista_tamano.push_back(V.SizeDomain(i));
    }
    G.PoblacionInicial(I, sujetos, lista_tamano);
  } else {
    for (int i = 0; i < G.N_individuos(); ++i) {
      G.PutModificado(i);
    }
  }

  fitness(ev);

  for (int ik = G.N_individuos() - 2; ik < G.N_individuos(); ++ik) {
    Obtener_Regla_Descriptiva(ev, ik, reglas_a_eliminar);
  }

  G.Sort();

  Fitness mejor_anterior = G.Valoracion(0);

  const int max_iter = 500;
  int sin_cambios = 0;
  while (sin_cambios <= max_iter) {
    if ((R.N_rule() > 1 && G.Valoracion(0)[0] <= 0 && Probabilidad(0.75)) ||
        (R.N_rule() > 1 && Probabilidad(0.01))) {
      // En el original elegía aquí una regla que luego no usaba. Se mantiene por el número que
      // pide.
      static_cast<void>(Select_Random_Individuo(R.N_rule(), -1, -1));
      G.MutacionUniforme_Estacionario_Especial();
    } else {
      G.CruceBasedLogical_Estacionario((1.0 * sin_cambios) / max_iter);
      G.MutacionUniforme_Estacionario();
    }
    fitness(ev);
    G.Sort_4L();

    if (G.Valoracion(0) == mejor_anterior) {
      ++sin_cambios;
      if (sin_cambios == 2) {
        const int t = 1;
        G.PutCodigo(t, G.Codigo(0));

        if (G.Valoracion(t)[0] != -E.N_Examples() || G.Valoracion(t)[1] != 1) {
          Obtener_Regla_Discriminante(ev, t, reglas_a_eliminar);
        } else {
          Obtener_Regla_Descriptiva(ev, t, reglas_a_eliminar);
        }
      }
    } else {
      sin_cambios = 0;
      mejor_anterior = G.Valoracion(0);
    }
  }

  const Fitness& mejor = G.Valoracion(0);
  const double menos_ne = -E.N_Examples();

  if (mejor[0] <= -1 && mejor[1] > 1 && mejor[1] >= (-mejor[0]) + 2) {
    progreso(Verbosidad::depuracion) << "->->->->->-> El nuevo caso especial\n";
  }

  if ((mejor[0] > -1) || (mejor[0] <= -1 && mejor[1] > 1 && mejor[1] >= (-mejor[0]) + 2) ||
      (mejor[0] == menos_ne && mejor[1] > 1) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] > 0) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] == 0 && mejor[3] > 0) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] == 0 && mejor[3] == 0 && mejor[4] > 0)) {
    genetcode code1;
    genetcode code2;
    Fitness valor{};
    std::array<double, 2> esp{};
    std::array<double, 2> gen{};
    double peso = 0;
    double nmas1 = 0;
    double nmas2 = 0;
    double nmenos1 = 0;
    double nmenos2 = 0;

    do {
      Obtener_Regla_Discriminante(ev, 0, reglas_a_eliminar);
      code1 = G.Codigo(0);
      Bondad(ev, 0, valor, reglas_a_eliminar);
      Weight_Individual(G, E, ev.tabla, 0, nmas1, nmenos1);
      gen = {valor[0], valor[1]};

      Obtener_Regla_Descriptiva(ev, 0, reglas_a_eliminar);
      Bondad(ev, 0, valor, reglas_a_eliminar);
      peso = Weight_Individual(G, E, ev.tabla, 0, nmas2, nmenos2);
      esp = {valor[0], valor[1]};
      code2 = G.Codigo(0);
    } while (esp != gen || !ComprobacionEspGen(code2.etiquetas, code1.etiquetas));

    R.Add(code2, nmas2, nmenos2, false);
    R_dis.Add(code1, nmas1, nmenos1, false);

    const std::string patron = Patron2Reglas(R, R_dis, V, R.N_rule() - 1);

    const std::string especifica = Patron1Regla(R, R.N_rule() - 1);
    const std::string general = Patron1Regla(R_dis, R_dis.N_rule() - 1);
    if (!ComprobacionEspGen(especifica, general)) {
      progreso(Verbosidad::depuracion) << "esp: \n" << especifica << '\n' << general << "\n:gen\n";
      Comparar2patronesPinta(especifica, general);
      Pausa();
    }

    code1.etiquetas = patron;
    R_mix.Add(code1, peso);

    Added_Rule = true;

    // Pongo la regla por defecto para la clase obtenida en la posición penultima de la poblacion
    code1.etiquetas.assign(code1.etiquetas.size(), '1');

    G.PutCodigo(0, code2);

    // Pongo la regla por defecto para otra clase en la ultima posicion de la poblacion
    code1.clase = (code1.clase + 1) % n_clases;
    G.PutCodigo(G.N_individuos() - 1, code1);
    code1.clase = (code1.clase + 1) % n_clases;
    G.PutCodigo(G.N_individuos() - 2, code1);
  }
}

void fitness(const Evaluacion& ev) {
  multipoblacion& G = ev.G;
  // Las reglas que eliminaría cada individuo solo interesan al evaluar la regla que se añade.
  std::vector<int> reglas_a_eliminar;
  Fitness valor{};
  for (int i = 0; i < G.N_individuos(); ++i) {
    if (G.Modificado(i)) {
      Bondad(ev, i, valor, reglas_a_eliminar);
      G.Valorar(i, valor);
    }
  }
}

void Obtener_Regla_Discriminante(const Evaluacion& ev, int elemento_i,
                                 std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  std::string& etiquetas = G.Etiquetas(elemento_i);
  std::vector<double>& niveles = G.Niveles(elemento_i);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  std::size_t tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto longitud = indice(V.Variable(static_cast<int>(j)).N_etiquetas());
    if (niveles[j] < umbral) {
      etiquetas.replace(tama, longitud, longitud, '1');
    }
    tama += longitud;
  }

  Fitness valor{};
  Fitness valor2{};
  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);

  // Quita cada atributo relevante si la regla mejora sin él.
  for (std::size_t j = 0; j < atributos; ++j) {
    if (niveles[j] >= umbral) {
      const double aux = niveles[j];
      niveles[j] = umbral - 0.001;

      Bondad(ev, elemento_i, valor2, reglas_a_eliminar);

      if (Mejor(valor2, valor)) {
        valor = valor2;
      } else {
        niveles[j] = aux;
      }
    }
  }

  // Admite cada etiqueta más si la regla no empeora.
  tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const double aux2 = niveles[j];
    const auto longitud = indice(V.Variable(static_cast<int>(j)).N_etiquetas());

    if (niveles[j] >= umbral) {
      for (std::size_t s = 0; s < longitud; ++s) {
        if (etiquetas[tama + s] == '0') {
          etiquetas[tama + s] = '1';
          Bondad(ev, elemento_i, valor2, reglas_a_eliminar);
          if (MejorOIgual(valor2, valor)) {
            valor = valor2;
          } else {
            etiquetas[tama + s] = '0';
            niveles[j] = aux2;
          }
        }
      }
    }
    tama += longitud;
  }

  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);
}

void Obtener_Regla_Descriptiva(const Evaluacion& ev, int elemento_i,
                               std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  std::string& etiquetas = G.Etiquetas(elemento_i);
  std::vector<double>& niveles = G.Niveles(elemento_i);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  Fitness valor{};
  Fitness valor2{};
  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);

  std::size_t tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto longitud = indice(V.Variable(static_cast<int>(j)).N_etiquetas());

    if (niveles[j] < umbral) {
      // Añade el atributo a la regla si, quitándole alguna etiqueta, no empeora.
      const double aux = niveles[j];
      bool mejora = false;
      niveles[j] = umbral;
      etiquetas.replace(tama, longitud, longitud, '1');

      for (std::size_t s = 0; s < longitud; ++s) {
        etiquetas[tama + s] = '0';
        Bondad(ev, elemento_i, valor2, reglas_a_eliminar);

        if (MejorOIgual(valor2, valor)) {
          valor = valor2;
          mejora = true;
        } else {
          etiquetas[tama + s] = '1';
          niveles[j] = umbral;
        }
      }

      if (!mejora) {
        niveles[j] = aux;
      }
    } else {
      // Quita cada etiqueta si la regla mejora sin ella.
      for (std::size_t s = 0; s < longitud; ++s) {
        if (etiquetas[tama + s] == '1') {
          etiquetas[tama + s] = '0';
          Bondad(ev, elemento_i, valor2, reglas_a_eliminar);
          if (Mejor(valor2, valor)) {
            valor = valor2;
          } else {
            etiquetas[tama + s] = '1';
          }
        }
      }
    }

    tama += longitud;
  }

  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);
}

double Weight_Individual(const multipoblacion& G, const example_set& E,
                         const TablaDeAdaptaciones& tabla, int elemento_i, double& n_mas,
                         double& n_menos) {
  const std::string& etiquetas = G.Etiquetas(elemento_i);
  const std::vector<double>& niveles = G.Niveles(elemento_i);
  const int clase = G.ClaseIndividuo(elemento_i);

  double positivos = 0;
  double negativos = 0;
  for (int j = 0; j < E.N_Examples(); ++j) {
    double aux_p = 0;
    double aux_n = 0;
    tabla.Consecuente(j, clase, aux_p, aux_n);
    const double x = tabla.Antecedente(j, etiquetas, niveles);
    positivos = positivos + (x * aux_p);
    negativos = negativos + (x * aux_n);
  }

  n_mas = positivos;
  n_menos = negativos;

  return (positivos + 1) / (positivos + negativos + 1);
}

double Bondad(const Evaluacion& ev, int elemento_i, Fitness& valor,
              std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  const example_set& E = ev.E;
  const ruleset& R = ev.R;
  const std::vector<int>& ponderacion_por_clase = ev.ponderacion_por_clase;

  const int ne = E.N_Examples();
  const int nrule = R.N_rule();
  const int clase = G.ClaseIndividuo(elemento_i);
  const int ponderacion = ponderacion_por_clase[indice(clase)];

  const std::string& etiquetas = G.Etiquetas(elemento_i);
  std::vector<double>& niveles = G.Niveles(elemento_i);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  std::size_t variables_irrelevantes = 0;
  int ceros_variables = 0;
  std::size_t pos = 0;  // posicion en el cromosoma nivel de valor

  for (std::size_t j = 0; j < atributos; ++j) {
    const auto tamanio = indice(V.Variable(static_cast<int>(j)).N_etiquetas());
    const std::string_view trozo = std::string_view(etiquetas).substr(pos, tamanio);
    if (niveles[j] < umbral) {
      ++variables_irrelevantes;
    } else if (Relevante(trozo)) {
      ceros_variables += static_cast<int>(std::count(trozo.begin(), trozo.end(), '0'));
    } else {
      niveles[j] = umbral - 0.001;
      ++variables_irrelevantes;
    }
    pos = pos + tamanio;
  }

  const int variables_relevantes = static_cast<int>(atributos - variables_irrelevantes);

  // Comprobar que es una regla valida
  if (!V.Es_Valida(etiquetas, niveles) || variables_irrelevantes >= atributos) {
    valor.fill(-2 * ne);
    return 0;
  }

  // Calculo previo para obtener el peso de la regla
  std::vector<double> adaptacion(indice(ne));
  double positivos = 0;
  double negativos = 0;
  double afecta = 0;
  for (int j = 0; j < ne; ++j) {
    double aux_p = 0;
    double aux_n = 0;
    ev.tabla.Consecuente(j, clase, aux_p, aux_n);

    const double a = ev.tabla.Antecedente(j, etiquetas, niveles);

    if (a * aux_p > 0) {
      ++afecta;
    }

    positivos = positivos + (a * aux_p);
    negativos = negativos + (a * aux_n);
    adaptacion[indice(j)] = aux_p < aux_n ? -a : a;
  }

  const double peso = (positivos + 1) / (positivos + negativos + 1);
  const bool menos_infinito = !(positivos > 0);

  if (ponderacion > 0) {
    afecta = afecta / ponderacion;
  }

  // Dado el peso de la regla obtener la bondad de la misma.
  std::vector<int> basica_esta_regla(indice(nrule));
  std::vector<int> errores_esta_regla(indice(nrule));
  const auto anota = [&](std::vector<int>& cuenta, int ejemplo, int aislada) {
    if (E.Rule_Aislada(ejemplo) == aislada && E.Rule_Used(ejemplo) >= 0 &&
        E.Rule_Used(ejemplo) < nrule) {
      ++cuenta[indice(E.Rule_Used(ejemplo))];
    }
  };
  double aciertos = 0;
  double fallos = 0;
  positivos = 0;
  negativos = 0;
  for (int j = 0; j < ne; ++j) {
    const double a = adaptacion[indice(j)];
    if (a > 0) {
      // La regla gana a la mejor de otra clase.
      if (a * peso > E.Grade_Is_Negative_Covered(j) ||
          (a * peso == E.Grade_Is_Negative_Covered(j) && peso > E.Peso_Is_Negative_Covered(j))) {
        if (!E.Is_Covered(j)) {
          positivos = positivos + (a * peso);
          ++aciertos;
          anota(errores_esta_regla, j, -1);
        } else {
          anota(basica_esta_regla, j, 1);
        }
      }
    } else if (a < 0) {
      // La regla gana a la mejor de la clase del ejemplo.
      if (-a * peso > E.Grade_Is_Positive_Covered(j) ||
          (-a * peso == E.Grade_Is_Positive_Covered(j) && peso > E.Peso_Is_Positive_Covered(j))) {
        negativos =
            negativos +
            ((-a * peso) / ponderacion_por_clase[indice(E.Class_of_example(V.Consecuente(), j))]);
        if (!E.Is_Covered(j)) {
          anota(errores_esta_regla, j, -1);
        } else {
          ++fallos;
          anota(basica_esta_regla, j, 1);
        }
      }
    }
  }

  // Evaluar si es posible eliminar alguna regla ya existente
  int reglas_eliminables = 0;
  double aciertos_adicionales = 0;
  int ganancia_en_variables = variables_relevantes;
  double diferencia_peso = peso;

  reglas_a_eliminar.clear();
  for (int i = ev.tam_rule_not_removed; i < nrule; ++i) {
    const std::size_t r = indice(i);
    const int ganancia = (basica_esta_regla[r] - ev.basica_para_regla[r]) +
                         (ev.errores_para_regla[r] - errores_esta_regla[r]);
    if (ganancia >= 0) {
      reglas_a_eliminar.push_back(i);
      ++reglas_eliminables;
      aciertos_adicionales += ganancia;
      ganancia_en_variables = R.Variables_of_rule(i) - ganancia_en_variables;
      diferencia_peso -= R.Get_Weight(i);
    }
  }

  positivos = ponderacion > 0 ? positivos / ponderacion : 0;

  if (aciertos + aciertos_adicionales - fallos > 0) {
    if (ponderacion > 0) {
      if (positivos - negativos < 0) {
        valor[0] = -0.9;
      } else {
        valor[0] = positivos - negativos;
        valor[0] *= peso * peso * peso * peso * peso * peso * peso * peso * peso;
      }
      if (positivos == 0 && menos_infinito) {
        valor[0] = -2 * ne;
      }
    } else {
      valor[0] = -2 * ne;
    }
  } else if (aciertos + aciertos_adicionales > 0) {
    valor[0] = aciertos + aciertos_adicionales - fallos;
    if (valor[0] == 0) {
      valor[0] = -ne;
    } else if (ev.reducir_reglas && !(reglas_eliminables >= (-valor[0]) + 2)) {
      valor[0] = -2 * ne;
    }
  } else {
    valor[0] = -(ne + fallos);
  }

  valor[6] = aciertos_adicionales;
  valor[5] = aciertos - fallos;
  valor[4] = 0;
  valor[3] = -variables_relevantes;
  valor[2] = afecta;
  valor[1] = reglas_eliminables;

  if (reglas_eliminables == 1 && (valor[0] == -ne || valor[0] == 0)) {
    valor[2] = aciertos_adicionales > 0 ? 0.1 + aciertos_adicionales : diferencia_peso;
    valor[3] = -ganancia_en_variables;
    valor[4] = 0;
    valor[5] = ceros_variables;
    // En el original, con una sola regla leía el peso de una regla que no existe: memoria sin
    // inicializar o de una regla ya quitada. No decide nada: dos individuos solo empatan hasta
    // aquí si los dos pasan por esta rama, y entonces leen el mismo valor (y forzándolo a 0, 1e9 y
    // -1e9, las reglas de los conjuntos de prueba no cambian).
    valor[6] = reglas_eliminables < nrule ? -R.Get_Weight(reglas_eliminables) : 0;
  }

  return peso;
}

std::string Patron1Regla(const ruleset& R_1, int pos) {
  // Sin esa regla, un patrón vacío. En el original leía la regla -1 cuando el genético no
  // añadía ninguna en la primera vuelta (R_mix vacío) y el programa reventaba (HALLAZGOS).
  if (pos < 0 || pos >= R_1.N_rule()) {
    return "";
  }
  return R_1.Get_Code_Rule(pos).etiquetas;
}

std::string Patron2Reglas(const ruleset& R_1, const ruleset& R_2, const VectorVar& V, int pos) {
  const std::string regla1 = EtiquetasEfectivas(R_1.Get_Code_Rule(pos), V);
  const std::string regla2 = EtiquetasEfectivas(R_2.Get_Code_Rule(pos), V);
  std::string regla3 = regla1;
  for (std::size_t i = 0; i < regla1.size(); ++i) {
    if (regla1[i] != regla2[i]) {
      regla3[i] = '?';
    }
  }
  return regla3;
}

double Aciertos(const ruleset& R, const VectorVar& V, example_set& E) {
  const int n = E.N_Examples();
  const int conse = V.Consecuente();
  double bien = 0;

  for (int i = 0; i < n; ++i) {
    const vectordouble w = E.Data(i);
    double g_pos_grado = 0;
    double peso_pos = 0;
    double g_neg_grado = 0;
    double peso_neg = 0;
    int regla_disparada = 0;
    int aislada = 0;
    const int clase = R.InferenceC(w, static_cast<int>(w.At(conse)), g_pos_grado, peso_pos,
                                   g_neg_grado, peso_neg, regla_disparada, aislada);
    const bool acierta = clase != -1 && clase == w.At(conse);
    if (acierta) {
      ++bien;
    }
    E.Mark(i, acierta, g_pos_grado, peso_pos, g_neg_grado, peso_neg, regla_disparada, aislada);
  }

  return bien / n;
}

std::vector<int> Reglas_Que_Disparan(const ruleset& R, const example_set& E) {
  std::vector<bool> usada(indice(R.N_rule()));

  for (int i = 0; i < E.N_Examples(); ++i) {
    double grado = 0;
    int regla_disparada = -1;
    if (R.InferenceC(E.Data(i), grado, regla_disparada) != -1) {
      usada[indice(regla_disparada)] = true;
    }
  }

  std::vector<int> lista;
  for (int i = 0; i < R.N_rule(); ++i) {
    if (usada[indice(i)]) {
      lista.push_back(i);
    }
  }
  return lista;
}

void Filtrar_Reglas_New(ruleset& R, ruleset& R_dis, ruleset& R_mix,
                        const std::vector<int>& reglas_a_eliminar) {
  for (auto i = reglas_a_eliminar.rbegin(); i != reglas_a_eliminar.rend(); ++i) {
    R.Remove(*i);
    R_dis.Remove(*i);
    R_mix.Remove(*i);
  }
}

std::vector<std::vector<double>> MedidasDeInformacion(const VectorVar& V, const example_set& E) {
  const int conse = V.Consecuente();
  const auto n_clases = indice(V.SizeDomain(conse));
  std::vector<std::vector<double>> I(indice(V.N_Antecedente()), std::vector<double>(n_clases + 1));

  const std::vector<double> py = CalcularProbabilidadSimpleVariable(V, E, conse);

  for (std::size_t i = 0; i < I.size(); ++i) {
    const int atributo = static_cast<int>(i);
    const std::vector<double> px = CalcularProbabilidadSimpleVariable(V, E, atributo);
    const std::vector<std::vector<double>> pxy =
        CalcularProbabilidadConjunta2Variables(V, E, atributo, conse);

    double I1 = 0;
    double H1 = 0;
    for (std::size_t k = 0; k < n_clases; ++k) {
      double I2 = 0;
      double H2 = 0;
      for (std::size_t q = 0; q < px.size(); ++q) {
        double aux = pxy[q][k] == 0 ? 0 : pxy[q][k] * std::log((px[q] * py[k]) / pxy[q][k]);
        I2 = I2 - aux;
        I1 = I1 - aux;

        aux = pxy[q][k] == 0 ? 0 : pxy[q][k] * std::log(pxy[q][k]);
        H2 = H2 - aux;
        H1 = H1 - aux;
      }
      I[i][k + 1] = H2 == 0 ? 0 : I2 / H2;
    }
    I[i][0] = H1 == 0 ? 0 : I1 / H1;

    V.PrintVar(atributo);
    progreso(Verbosidad::depuracion) << ": " << I1 << "/" << H1 << ": " << I[i][0] << '\n';
  }
  return I;
}

std::vector<Regla> Aprender(const VectorVar& V, const example_set& E, const Opciones& options) {
  // El informe (el antiguo <tarea>.res) solo se escribe si se pide.
  const bool informe = !options.informe.empty();
  const std::string task_res = options.informe.string();

  V.Pinta();

  if (informe) {
    std::ofstream out(task_res);
    out << "Experiment: " << task_res << '\n';
    const std::time_t ahora = std::time(nullptr);
    out << std::ctime(&ahora);  // NOLINT(concurrency-mt-unsafe)
    out << "===================================================\n\n";
  }

  std::srand(options.semilla);
  progreso(Verbosidad::depuracion) << "SEED " << options.semilla << '\n';

  // El tamano del nivel de valor y el numero de clases
  int tama_valor = 0;
  int n_clases = 0;
  V.Encode(tama_valor, n_clases);
  // El nivel de variable: el de cada atributo y el umbral
  const int tama_nivel = V.N_Antecedente() + 1;

  progreso(Verbosidad::detalle) << "Calculando medidas de informacion\n";
  const std::vector<std::vector<double>> Med_Inform =
      MedidasDeInformacion(V, E.N_Examples() > 10000 ? E.SubSet(1000) : E);
  progreso(Verbosidad::detalle) << "Terminado el calculo de las medidas de informacion\n";

  ruleset R;
  R.AddDomain(V);
  ruleset R_dis = R;
  ruleset R_prev = R;
  ruleset R_prev_dis = R;
  ruleset R_mix = R;
  ruleset R_mix_prev = R;

  multipoblacion G(n_clases, n_clases * 10, tama_valor, tama_nivel, 2.0 / tama_valor, 0, 0.01);

  const int limite = E.N_Examples();
  const int paso = static_cast<int>(options.tamano_episodio);
  // Para la línea de cada episodio (Verbosidad::episodios).
  const int total_episodios = (limite + paso - 1) / paso;
  // En cuántos episodios ha aprendido reglas, más uno; y el episodio actual (que empieza en
  // it2 * paso).
  int it = 1;
  int it2 = 0;
  const double percent = 0.10;
  double umbral_acierto = 0.7;
  double acumulado = 0.0;
  double accuracy_old = 0;
  double acierto_anterior = 0;
  int inicio = 0;
  bool salir = false;
  const std::clock_t tiempo0 = std::clock();
  example_set E_Par;

  while (!salir) {
    int fin = inicio + paso;

    if (fin >= limite) {
      fin = limite;
      salir = true;
    }

    E_Par = E.SubSet(inicio, fin - 1);

    progreso(Verbosidad::detalle)
        << "\n=================================================== Episodio de " << inicio << " a "
        << fin << " ====================================\n";
    const int episodio = it2 + 1;
    const int primer_ejemplo = inicio;
    const int ultimo_ejemplo = fin - 1;
    const int reglas_al_empezar = R_prev.N_rule();
    const auto empieza_el_episodio = std::chrono::steady_clock::now();

    const double consumido = static_cast<double>(std::clock() - tiempo0) / CLOCKS_PER_SEC;
    progreso(Verbosidad::detalle) << "===================== Tiempo Consumido: " << consumido
                                  << "      ETA: "
                                  << (consumido / (it2 + 1)) * (total_episodios - it2) << '\n';
    progreso(Verbosidad::detalle) << "=================================================== Tamano "
                                  << E_Par.N_Examples()
                                  << " ====================================\n\n";
    progreso(Verbosidad::depuracion) << "[Paso: " << paso << "]\n";
    inicio = fin;

    progreso(Verbosidad::depuracion) << "Creando la tabla de adaptaciones.....\n";
    const TablaDeAdaptaciones Tabla(V, E_Par);
    progreso(Verbosidad::depuracion) << "Tabla de adaptaciones creada!\n";

    progreso(Verbosidad::detalle) << "  Numero de Reglas: " << R_prev.N_rule() << "("
                                  << R_prev.N_rule(0.5) << ")\n";

    const double last_accuracy = accuracy_old;
    acumulado += last_accuracy;
    if (acumulado == 0) {
      umbral_acierto = 1;
    }
    progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el anterior conjunto de ejemplos : " << last_accuracy
        << '\n';

    accuracy_old = Aciertos(R_prev, V, E_Par);
    progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el nuevo conjunto de ejemplos : " << accuracy_old
        << '\n';
    const double acierto_al_empezar = accuracy_old;

    // El umbral sale del acierto sobre un episodio anterior: el de hace 10 cada 10 episodios y el
    // primero hasta el décimo. Los dos primeros episodios, sin anterior que mirar, 1.
    if (it2 >= 10 && it2 % 10 == 0) {
      example_set E_Anteriores = E.SubSet((it2 - 10) * paso, (it2 - 9) * paso - 1);
      const double acierto = Aciertos(R_prev, V, E_Anteriores);
      umbral_acierto = acierto - percent * acierto;
    } else if (it2 < 10 && it2 > 1) {
      example_set E_Anteriores = E.SubSet(0, paso - 1);
      const double acierto = Aciertos(R_prev, V, E_Anteriores);
      umbral_acierto = acierto - percent * acierto;
    } else if (it2 < 10) {
      umbral_acierto = 1;
    }

    ++it2;

    progreso(Verbosidad::detalle) << "Diferencia de acierto entre los dos últimos episodios: "
                                  << acierto_anterior - last_accuracy << '\n';
    acierto_anterior = last_accuracy;

    progreso(Verbosidad::detalle) << "En la iteración " << it << " el umbral se ha fijado a "
                                  << umbral_acierto << " (" << umbral_acierto * (1 + percent)
                                  << ")\n";

    R_prev.FixModifyToTrue();
    R_prev_dis.FixModifyToTrue();

    ActualizarPeso(G, E_Par, R_prev, Tabla);
    ActualizarPeso(G, E_Par, R_prev_dis, Tabla);

    R_prev_dis.Recalculate_Weight();
    R_prev.Recalculate_Weight();

    accuracy_old = Aciertos(R_prev, V, E_Par);
    progreso(Verbosidad::detalle) << "Aciertos despues de ajustar los pesos de las reglas: "
                                  << accuracy_old << '\n';
    const double acierto_despuesAjustarPeso = accuracy_old;

    if (accuracy_old < umbral_acierto) {
      example_set E_Par_Not_Covered = E_Par;

      for (const int regla : Reglas_Que_Disparan(R_prev, E_Par_Not_Covered)) {
        R.Add(R_prev.Get_Code_Rule(regla), R_prev.Get_N_Mas(regla), R_prev.Get_N_Menos(regla),
              false);

        const double nmas = R_prev_dis.Get_N_Mas(regla);
        const double nmenos = R_prev_dis.Get_N_Menos(regla);
        R_dis.Add(R_prev_dis.Get_Code_Rule(regla), nmas, nmenos, false);
        R_mix.Add(R_mix_prev.Get_Code_Rule(regla), nmas, nmenos, false);
      }

      progreso(Verbosidad::detalle)
          << "Reglas no filtradas: " << R.N_rule() << "(" << R.N_rule(0.5) << ")\n";

      const TablaDeAdaptaciones Tabla1(V, E_Par_Not_Covered);

      progreso(Verbosidad::detalle)
          << "\n-------------------------------------------> Nuevo numero de ejemplos: "
          << E_Par_Not_Covered.N_Examples() << '\n';

      accuracy_old = Aciertos(R, V, E_Par_Not_Covered);
      progreso(Verbosidad::detalle)
          << "Aciertos de la descriptiva antes de lanzar el genetico: " << accuracy_old << '\n';

      const int initial_size_of_rule = R.N_rule();
      std::string last_patron;
      bool primera_vez = true;
      bool reducir_reglas = false;
      bool Added_Rule = false;
      std::vector<int> reglas_a_eliminar;

      do {
        const int anterior_n = E_Par_Not_Covered.Not_Covered_Examples();
        const std::vector<int> ejemplos_por_clase =
            E_Par_Not_Covered.Examples_per_Class(V.Consecuente(), n_clases);

        std::vector<int> basica_para_regla(indice(R.N_rule() + 1));
        std::vector<int> errores_para_regla(indice(R.N_rule() + 1));
        Obtener_Aciertos_y_Errores_Reglas_Aprendidas(E_Par_Not_Covered, basica_para_regla,
                                                     errores_para_regla);

        const Evaluacion ev{G,
                            V,
                            E_Par_Not_Covered,
                            R,
                            reducir_reglas,
                            ejemplos_por_clase,
                            Tabla1,
                            basica_para_regla,
                            errores_para_regla,
                            initial_size_of_rule};
        GA(ev, R, R_dis, R_mix, Med_Inform, primera_vez, reglas_a_eliminar, Added_Rule);

        primera_vez = false;

        Aciertos(R, V, E_Par_Not_Covered);
        const std::string current_patron = Patron1Regla(R_mix, R_mix.N_rule() - 1);
        if (current_patron == last_patron) {
          Added_Rule = false;
        }
        last_patron = current_patron;

        if (Added_Rule && !reglas_a_eliminar.empty()) {
          Filtrar_Reglas_New(R, R_dis, R_mix, reglas_a_eliminar);
          Aciertos(R, V, E_Par_Not_Covered);
        }

        if (G.Valoracion(0)[0] > 0 && anterior_n == E_Par_Not_Covered.Not_Covered_Examples()) {
          progreso(Verbosidad::detalle) << "+++++++++++++++++++++ ERROR ++++++++++++++++++++++\n";
        }

        if (G.Valoracion(0)[0] != -2 * E_Par_Not_Covered.N_Examples() && !reducir_reglas &&
            !Added_Rule) {
          reducir_reglas = true;
          Added_Rule = true;
        }
      } while (Added_Rule);

      ++it;

      if (it2 > 1) {
        const example_set E_Anteriores = E.SubSet(it2 > 10 ? (it2 - 10) * paso : 0, inicio - 1);
        const TablaDeAdaptaciones Tabla2(V, E_Anteriores);
        ActualizarPeso(G, E_Anteriores, R, Tabla2);
        ActualizarPeso(G, E_Anteriores, R_dis, Tabla2);
      }

      R_prev.AddRuleset(R);
      R_prev_dis.AddRuleset(R_dis);
      R_mix_prev.AddRuleset(R_mix);
      progreso(Verbosidad::detalle)
          << "Aniadidas " << R.N_rule() - initial_size_of_rule << " reglas\n";

      accuracy_old = Aciertos(R_prev, V, E_Par);
      progreso(Verbosidad::detalle)
          << "Acierto antes de analizar patrones: " << accuracy_old << '\n';

      if (acierto_despuesAjustarPeso < accuracy_old) {
        progreso(Verbosidad::detalle) << "............................. Analizando patrones "
                                         "entre reglas .......................\n";
        AnalizarConjuntosDeReglas(V, R_prev_dis, R_prev, R_mix_prev, R.N_rule());
        accuracy_old = Aciertos(R_prev, V, E_Par);
        progreso(Verbosidad::detalle)
            << "Acierto despues de analizar patrones: " << accuracy_old << '\n';
      } else {
        for (int i = 0; i < R.N_rule(); ++i) {
          R_prev.Remove(0);
          R_prev_dis.Remove(0);
          R_mix_prev.Remove(0);
        }
        accuracy_old = Aciertos(R_prev, V, E_Par);
      }
    }

    // Una línea por episodio. Solo con lo que ya se ha calculado: Aciertos marca los ejemplos y
    // llamarlo aquí cambiaría las reglas.
    if (se_escribe(Verbosidad::episodios)) {
      const std::chrono::duration<double> segundos =
          std::chrono::steady_clock::now() - empieza_el_episodio;
      progreso(Verbosidad::episodios) << std::format(
          "INSLV: episodio {} de {} (ejemplos {} a {}): reglas {} -> {}, acierto {:.3f} -> "
          "{:.3f}, {:.2f} s\n",
          episodio, total_episodios, primer_ejemplo, ultimo_ejemplo, reglas_al_empezar,
          R_prev.N_rule(), acierto_al_empezar, accuracy_old, segundos.count());
    }
    if (informe) {
      std::ofstream salida(task_res, std::ios::app);
      salida << "*****************************************************************\n";
      for (int i = 0; i < R.N_rule(); ++i) {
        salida << i << ") " << R.SPrint(i) << '\n';
      }
      salida << '\n';
    }

    while (R.N_rule() > 0) {
      R.Remove();
      R_dis.Remove();
      R_mix.Remove();
    }
  }

  if (informe) {
    Escribir_Informe(task_res, V, E_Par, R_prev, R_prev_dis, R_mix_prev,
                     static_cast<double>(std::clock() - tiempo0) / CLOCKS_PER_SEC);
  }

  // Las descriptivas (R_prev) o sus versiones discriminantes (R_prev_dis), emparejadas regla a
  // regla.
  const ruleset& elegidas = options.reglas == TipoDeReglas::discriminantes ? R_prev_dis : R_prev;
  std::vector<Regla> reglas;
  for (int i = 0; i < elegidas.N_rule(); ++i) {
    Regla regla;
    regla.antecedente = elegidas.Antecedente(i);
    regla.consecuente = indice(elegidas.Class_of_rule(i));
    regla.cubiertos = elegidas.Get_N_Mas(i);
    regla.peso = elegidas.Get_Weight(i);
    reglas.push_back(regla);
  }
  return reglas;
}

}  // namespace inslv
