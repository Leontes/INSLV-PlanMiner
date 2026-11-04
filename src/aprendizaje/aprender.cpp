// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/aprender.h"

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "inslv/aprendizaje/bondad.h"
#include "inslv/aprendizaje/ga.h"
#include "inslv/aprendizaje/medidas.h"
#include "inslv/aprendizaje/patrones.h"
#include "inslv/aprendizaje/tabla.h"
#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/codigo.hpp"
#include "inslv/genetico/multipoblacion.hpp"
#include "inslv/reglas/ruleset.h"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Por regla, los ejemplos que solo ella clasifica bien y los que clasifica mal por poco.
void Obtener_Aciertos_y_Errores_Reglas_Aprendidas(const datos::Ejemplos& E,
                                                  std::vector<int>& basica_para_regla,
                                                  std::vector<int>& errores_para_regla) {
  for (int j = 0; j < E.numero_de_ejemplos(); ++j) {
    if (E.regla_aislada(j) == 1 && E.regla_usada(j) >= 0) {
      ++basica_para_regla[indice(E.regla_usada(j))];
    } else if (E.regla_aislada(j) == -1 && E.regla_usada(j) >= 0) {
      ++errores_para_regla[indice(E.regla_usada(j))];
    }
  }
}

// Los aciertos de cada regla, para el informe (marca los ejemplos, como Aciertos).
double Aciertos_Por_Regla(const ruleset& R, const difuso::Variables& V, datos::Ejemplos& E,
                          const std::string& nomfich) {
  const double acierto = Aciertos(R, V, E);
  std::vector<int> bien_clas(indice(R.N_rule()));
  std::vector<int> mal_clas(indice(R.N_rule()));
  int bien = 0;
  int mal = 0;
  int nosesabe = 0;
  for (int i = 0; i < E.numero_de_ejemplos(); ++i) {
    if (E.regla_usada(i) == -1) {
      ++nosesabe;
    } else if (E.bien_clasificado(i)) {
      ++bien;
      ++bien_clas[indice(E.regla_usada(i))];
    } else {
      ++mal;
      ++mal_clas[indice(E.regla_usada(i))];
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

void Escribir_Informe(const std::string& task_res, const difuso::Variables& V, datos::Ejemplos& E,
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

  out << "Frecuencia de aparicion de las " << V.numero_de_atributos()
      << " distintas variables en las reglas: \n";
  for (int i = 0; i < V.numero_de_atributos(); ++i) {
    out << V.nombre(i) << ": --> "
        << (1.0 * frecuencia_variables[indice(i)] / R_prev.N_rule()) * 100 << " %\n";
  }
}

}  // namespace

double Aciertos(const ruleset& R, const difuso::Variables& V, datos::Ejemplos& E) {
  const int n = E.numero_de_ejemplos();
  const int conse = V.consecuente();
  double bien = 0;

  for (int i = 0; i < n; ++i) {
    const datos::Ejemplo w = E.ejemplo(i);
    double g_pos_grado = 0;
    double peso_pos = 0;
    double g_neg_grado = 0;
    double peso_neg = 0;
    int regla_disparada = 0;
    int aislada = 0;
    const int clase = R.InferenceC(w, static_cast<int>(w.valor(conse)), g_pos_grado, peso_pos,
                                   g_neg_grado, peso_neg, regla_disparada, aislada);
    const bool acierta = clase != -1 && clase == w.valor(conse);
    if (acierta) {
      ++bien;
    }
    E.anotar(i, acierta, g_pos_grado, peso_pos, g_neg_grado, peso_neg, regla_disparada, aislada);
  }

  return bien / n;
}

std::vector<int> Reglas_Que_Disparan(const ruleset& R, const datos::Ejemplos& E) {
  std::vector<bool> usada(indice(R.N_rule()));

  for (int i = 0; i < E.numero_de_ejemplos(); ++i) {
    double grado = 0;
    int regla_disparada = -1;
    if (R.InferenceC(E.ejemplo(i), grado, regla_disparada) != -1) {
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

void ActualizarPeso(genetico::MultiPoblacion& G, const datos::Ejemplos& E, ruleset& R,
                    const TablaDeAdaptaciones& tabla) {
  double n_mas = 0;
  double n_menos = 0;
  for (int i = 0; i < R.N_rule(); ++i) {
    if (R.Get_Modificada(i)) {
      G.poner_codigo(G.numero_de_individuos() - 1, R.Get_Code_Rule(i));
      Weight_Individual(G, E, tabla, G.numero_de_individuos() - 1, n_mas, n_menos);
      R.Add_Weight(i, n_mas, n_menos);
      R.Modificada(i, false);
    }
  }
}

std::vector<Regla> Aprender(const difuso::Variables& V, const datos::Ejemplos& E,
                            const Opciones& options) {
  // El informe (el antiguo <tarea>.res) solo se escribe si se pide.
  const bool informe = !options.informe.empty();
  const std::string task_res = options.informe.string();

  V.pintar();

  if (informe) {
    std::ofstream out(task_res);
    out << "Experiment: " << task_res << '\n';
    const std::time_t ahora = std::time(nullptr);
    out << std::ctime(&ahora);  // NOLINT(concurrency-mt-unsafe)
    out << "===================================================\n\n";
  }

  std::srand(options.semilla);
  comun::progreso(Verbosidad::depuracion) << "SEED " << options.semilla << '\n';

  const int tama_valor = V.etiquetas_de_atributos();
  const int n_clases = V.numero_de_etiquetas(V.consecuente());
  // El nivel de variable: el de cada atributo y el umbral
  const int tama_nivel = V.numero_de_atributos() + 1;

  comun::progreso(Verbosidad::detalle) << "Calculando medidas de informacion\n";
  const std::vector<std::vector<double>> Med_Inform =
      MedidasDeInformacion(V, E.numero_de_ejemplos() > 10000 ? E.primeros(1000) : E);
  comun::progreso(Verbosidad::detalle) << "Terminado el calculo de las medidas de informacion\n";

  ruleset R;
  R.AddDomain(V);
  ruleset R_dis = R;
  ruleset R_prev = R;
  ruleset R_prev_dis = R;
  ruleset R_mix = R;
  ruleset R_mix_prev = R;

  genetico::MultiPoblacion G(n_clases, n_clases * 10, tama_valor, tama_nivel, 2.0 / tama_valor, 0,
                             0.01);

  const int limite = E.numero_de_ejemplos();
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
  datos::Ejemplos E_Par;

  while (!salir) {
    int fin = inicio + paso;

    if (fin >= limite) {
      fin = limite;
      salir = true;
    }

    E_Par = E.subconjunto(inicio, fin - 1);

    comun::progreso(Verbosidad::detalle)
        << "\n=================================================== Episodio de " << inicio << " a "
        << fin << " ====================================\n";
    const int episodio = it2 + 1;
    const int primer_ejemplo = inicio;
    const int ultimo_ejemplo = fin - 1;
    const int reglas_al_empezar = R_prev.N_rule();
    const auto empieza_el_episodio = std::chrono::steady_clock::now();

    const double consumido = static_cast<double>(std::clock() - tiempo0) / CLOCKS_PER_SEC;
    comun::progreso(Verbosidad::detalle)
        << "===================== Tiempo Consumido: " << consumido
        << "      ETA: " << (consumido / (it2 + 1)) * (total_episodios - it2) << '\n';
    comun::progreso(Verbosidad::detalle)
        << "=================================================== Tamano "
        << E_Par.numero_de_ejemplos() << " ====================================\n\n";
    comun::progreso(Verbosidad::depuracion) << "[Paso: " << paso << "]\n";
    inicio = fin;

    comun::progreso(Verbosidad::depuracion) << "Creando la tabla de adaptaciones.....\n";
    const TablaDeAdaptaciones Tabla(V, E_Par);
    comun::progreso(Verbosidad::depuracion) << "Tabla de adaptaciones creada!\n";

    comun::progreso(Verbosidad::detalle)
        << "  Numero de Reglas: " << R_prev.N_rule() << "(" << R_prev.N_rule(0.5) << ")\n";

    const double last_accuracy = accuracy_old;
    acumulado += last_accuracy;
    if (acumulado == 0) {
      umbral_acierto = 1;
    }
    comun::progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el anterior conjunto de ejemplos : " << last_accuracy
        << '\n';

    accuracy_old = Aciertos(R_prev, V, E_Par);
    comun::progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el nuevo conjunto de ejemplos : " << accuracy_old
        << '\n';
    const double acierto_al_empezar = accuracy_old;

    // El umbral sale del acierto sobre un episodio anterior: el de hace 10 cada 10 episodios y el
    // primero hasta el décimo. Los dos primeros episodios, sin anterior que mirar, 1.
    if (it2 >= 10 && it2 % 10 == 0) {
      datos::Ejemplos E_Anteriores = E.subconjunto((it2 - 10) * paso, (it2 - 9) * paso - 1);
      const double acierto = Aciertos(R_prev, V, E_Anteriores);
      umbral_acierto = acierto - percent * acierto;
    } else if (it2 < 10 && it2 > 1) {
      datos::Ejemplos E_Anteriores = E.subconjunto(0, paso - 1);
      const double acierto = Aciertos(R_prev, V, E_Anteriores);
      umbral_acierto = acierto - percent * acierto;
    } else if (it2 < 10) {
      umbral_acierto = 1;
    }

    ++it2;

    comun::progreso(Verbosidad::detalle)
        << "Diferencia de acierto entre los dos últimos episodios: "
        << acierto_anterior - last_accuracy << '\n';
    acierto_anterior = last_accuracy;

    comun::progreso(Verbosidad::detalle)
        << "En la iteración " << it << " el umbral se ha fijado a " << umbral_acierto << " ("
        << umbral_acierto * (1 + percent) << ")\n";

    R_prev.FixModifyToTrue();
    R_prev_dis.FixModifyToTrue();

    ActualizarPeso(G, E_Par, R_prev, Tabla);
    ActualizarPeso(G, E_Par, R_prev_dis, Tabla);

    R_prev_dis.Recalculate_Weight();
    R_prev.Recalculate_Weight();

    accuracy_old = Aciertos(R_prev, V, E_Par);
    comun::progreso(Verbosidad::detalle)
        << "Aciertos despues de ajustar los pesos de las reglas: " << accuracy_old << '\n';
    const double acierto_despuesAjustarPeso = accuracy_old;

    if (accuracy_old < umbral_acierto) {
      datos::Ejemplos E_Par_Not_Covered = E_Par;

      for (const int regla : Reglas_Que_Disparan(R_prev, E_Par_Not_Covered)) {
        R.Add(R_prev.Get_Code_Rule(regla), R_prev.Get_N_Mas(regla), R_prev.Get_N_Menos(regla),
              false);

        const double nmas = R_prev_dis.Get_N_Mas(regla);
        const double nmenos = R_prev_dis.Get_N_Menos(regla);
        R_dis.Add(R_prev_dis.Get_Code_Rule(regla), nmas, nmenos, false);
        R_mix.Add(R_mix_prev.Get_Code_Rule(regla), nmas, nmenos, false);
      }

      comun::progreso(Verbosidad::detalle)
          << "Reglas no filtradas: " << R.N_rule() << "(" << R.N_rule(0.5) << ")\n";

      const TablaDeAdaptaciones Tabla1(V, E_Par_Not_Covered);

      comun::progreso(Verbosidad::detalle)
          << "\n-------------------------------------------> Nuevo numero de ejemplos: "
          << E_Par_Not_Covered.numero_de_ejemplos() << '\n';

      accuracy_old = Aciertos(R, V, E_Par_Not_Covered);
      comun::progreso(Verbosidad::detalle)
          << "Aciertos de la descriptiva antes de lanzar el genetico: " << accuracy_old << '\n';

      const int initial_size_of_rule = R.N_rule();
      std::string last_patron;
      bool primera_vez = true;
      bool reducir_reglas = false;
      bool Added_Rule = false;
      std::vector<int> reglas_a_eliminar;

      do {
        const int anterior_n = E_Par_Not_Covered.sin_clasificar_bien();
        const std::vector<int> ejemplos_por_clase =
            E_Par_Not_Covered.sin_clasificar_bien_por_clase(V.consecuente(), n_clases);

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

        if (G.valoracion(0)[0] > 0 && anterior_n == E_Par_Not_Covered.sin_clasificar_bien()) {
          comun::progreso(Verbosidad::detalle)
              << "+++++++++++++++++++++ ERROR ++++++++++++++++++++++\n";
        }

        if (G.valoracion(0)[0] != -2 * E_Par_Not_Covered.numero_de_ejemplos() && !reducir_reglas &&
            !Added_Rule) {
          reducir_reglas = true;
          Added_Rule = true;
        }
      } while (Added_Rule);

      ++it;

      if (it2 > 1) {
        const datos::Ejemplos E_Anteriores =
            E.subconjunto(it2 > 10 ? (it2 - 10) * paso : 0, inicio - 1);
        const TablaDeAdaptaciones Tabla2(V, E_Anteriores);
        ActualizarPeso(G, E_Anteriores, R, Tabla2);
        ActualizarPeso(G, E_Anteriores, R_dis, Tabla2);
      }

      R_prev.AddRuleset(R);
      R_prev_dis.AddRuleset(R_dis);
      R_mix_prev.AddRuleset(R_mix);
      comun::progreso(Verbosidad::detalle)
          << "Aniadidas " << R.N_rule() - initial_size_of_rule << " reglas\n";

      accuracy_old = Aciertos(R_prev, V, E_Par);
      comun::progreso(Verbosidad::detalle)
          << "Acierto antes de analizar patrones: " << accuracy_old << '\n';

      if (acierto_despuesAjustarPeso < accuracy_old) {
        comun::progreso(Verbosidad::detalle) << "............................. Analizando patrones "
                                                "entre reglas .......................\n";
        AnalizarConjuntosDeReglas(V, R_prev_dis, R_prev, R_mix_prev, R.N_rule());
        accuracy_old = Aciertos(R_prev, V, E_Par);
        comun::progreso(Verbosidad::detalle)
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
    if (comun::se_escribe(Verbosidad::episodios)) {
      const std::chrono::duration<double> segundos =
          std::chrono::steady_clock::now() - empieza_el_episodio;
      comun::progreso(Verbosidad::episodios) << std::format(
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
