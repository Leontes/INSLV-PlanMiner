// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/aprender.hpp"

#include <chrono>
#include <cstddef>
#include <ctime>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "inslv/aprendizaje/bondad.hpp"
#include "inslv/aprendizaje/busqueda.hpp"
#include "inslv/aprendizaje/medidas.hpp"
#include "inslv/aprendizaje/patrones.hpp"
#include "inslv/aprendizaje/tabla.hpp"
#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/codigo.hpp"
#include "inslv/genetico/multipoblacion.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Por regla, los ejemplos que solo ella clasifica bien y los que clasifica mal por poco.
void contar_basicos_y_errores(const datos::Ejemplos& ejemplos, std::vector<int>& basica_para_regla,
                              std::vector<int>& errores_para_regla) {
  for (int j = 0; j < ejemplos.numero_de_ejemplos(); ++j) {
    if (ejemplos.regla_aislada(j) == 1 && ejemplos.regla_usada(j) >= 0) {
      ++basica_para_regla[indice(ejemplos.regla_usada(j))];
    } else if (ejemplos.regla_aislada(j) == -1 && ejemplos.regla_usada(j) >= 0) {
      ++errores_para_regla[indice(ejemplos.regla_usada(j))];
    }
  }
}

// Los aciertos de cada regla, para el informe (marca los ejemplos, como aciertos).
double aciertos_por_regla(const reglas::ConjuntoDeReglas& reglas,
                          const difuso::Variables& variables, datos::Ejemplos& ejemplos,
                          const std::string& fichero) {
  const double acierto = aciertos(reglas, variables, ejemplos);
  std::vector<int> bien_por_regla(indice(reglas.numero_de_reglas()));
  std::vector<int> mal_por_regla(indice(reglas.numero_de_reglas()));
  int bien = 0;
  int mal = 0;
  int sin_clase = 0;
  for (int i = 0; i < ejemplos.numero_de_ejemplos(); ++i) {
    if (ejemplos.regla_usada(i) == -1) {
      ++sin_clase;
    } else if (ejemplos.bien_clasificado(i)) {
      ++bien;
      ++bien_por_regla[indice(ejemplos.regla_usada(i))];
    } else {
      ++mal;
      ++mal_por_regla[indice(ejemplos.regla_usada(i))];
    }
  }

  std::ofstream out(fichero, std::ios::app);
  out << "Aciertos: " << bien << '\n';
  out << "Errores:  " << mal << '\n';
  out << "No clasificados: " << sin_clase << '\n';
  for (int i = 0; i < reglas.numero_de_reglas(); ++i) {
    out << "\tRegla " << i << "(" << reglas.texto_del_consecuente(i)
        << "): " << bien_por_regla[indice(i)] << " / " << mal_por_regla[indice(i)] << '\n';
  }
  return acierto;
}

void escribir_informe(const std::string& fichero, const difuso::Variables& variables,
                      datos::Ejemplos& ejemplos, const reglas::ConjuntoDeReglas& descriptivas,
                      const reglas::ConjuntoDeReglas& discriminantes,
                      const reglas::ConjuntoDeReglas& patrones, double tiempo) {
  const double aciertos_discriminantes =
      aciertos_por_regla(discriminantes, variables, ejemplos, fichero);
  const double aciertos_descriptivas =
      aciertos_por_regla(descriptivas, variables, ejemplos, fichero);

  std::vector<int> frecuencia;
  const double variables_usadas = descriptivas.frecuencia_de_atributos(frecuencia);

  std::ofstream out(fichero, std::ios::app);
  out << "----------------------------------------------------\n";
  out << "Numero de reglas: " << descriptivas.numero_de_reglas() << '\n';
  out << "Variables por regla: " << descriptivas.atributos_por_regla() << '\n';
  out << "Variables usadas: " << variables_usadas << '\n';
  out << "Tiempo: " << tiempo << '\n';
  out << "Condiciones: " << descriptivas.numero_de_condiciones() << '\n';
  out << "Aciertos Discriminantes: " << aciertos_discriminantes << '\n';
  out << "Aciertos Descriptivas: " << aciertos_descriptivas << '\n';
  out << "----------------------------------------------------\n";
  out << "------------------- REGLAS -------------------------\n";
  out << "----------------------------------------------------\n";

  out << "Descriptivas: \n";
  out.close();
  descriptivas.anadir_a_fichero(fichero);
  out.open(fichero, std::ios::app);
  out << "Predictivas: \n";
  out.close();
  discriminantes.anadir_a_fichero(fichero);
  out.open(fichero, std::ios::app);
  out << "Patrones: \n";

  for (int i = 0; i < patrones.numero_de_reglas(); ++i) {
    out << patron_de_regla(patrones, i) << '(' << patrones.clase(i) << ")\n";
  }

  out << "Frecuencia de aparicion de las " << variables.numero_de_atributos()
      << " distintas variables en las reglas: \n";
  for (int i = 0; i < variables.numero_de_atributos(); ++i) {
    out << variables.nombre(i) << ": --> "
        << (1.0 * frecuencia[indice(i)] / descriptivas.numero_de_reglas()) * 100 << " %\n";
  }
}

}  // namespace

double aciertos(const reglas::ConjuntoDeReglas& reglas, const difuso::Variables& variables,
                datos::Ejemplos& ejemplos) {
  const int n = ejemplos.numero_de_ejemplos();
  const int clase_variable = variables.consecuente();
  double bien = 0;

  for (int i = 0; i < n; ++i) {
    const datos::Ejemplo ejemplo = ejemplos.ejemplo(i);
    double g_pos_grado = 0;
    double peso_pos = 0;
    double g_neg_grado = 0;
    double peso_neg = 0;
    int regla_disparada = 0;
    int aislada = 0;
    const int clase =
        reglas.inferir(ejemplo, static_cast<int>(ejemplo.valor(clase_variable)), g_pos_grado,
                       peso_pos, g_neg_grado, peso_neg, regla_disparada, aislada);
    const bool acierta = clase != -1 && clase == ejemplo.valor(clase_variable);
    if (acierta) {
      ++bien;
    }
    ejemplos.anotar(i, acierta, g_pos_grado, peso_pos, g_neg_grado, peso_neg, regla_disparada,
                    aislada);
  }

  return bien / n;
}

std::vector<int> reglas_que_disparan(const reglas::ConjuntoDeReglas& reglas,
                                     const datos::Ejemplos& ejemplos) {
  std::vector<bool> usada(indice(reglas.numero_de_reglas()));

  for (int i = 0; i < ejemplos.numero_de_ejemplos(); ++i) {
    double grado = 0;
    int regla_disparada = -1;
    if (reglas.inferir(ejemplos.ejemplo(i), grado, regla_disparada) != -1) {
      usada[indice(regla_disparada)] = true;
    }
  }

  std::vector<int> lista;
  for (int i = 0; i < reglas.numero_de_reglas(); ++i) {
    if (usada[indice(i)]) {
      lista.push_back(i);
    }
  }
  return lista;
}

void actualizar_pesos(genetico::MultiPoblacion& poblacion, const datos::Ejemplos& ejemplos,
                      reglas::ConjuntoDeReglas& reglas, const TablaDeAdaptaciones& tabla) {
  double aciertos_de_la_regla = 0;
  double fallos_de_la_regla = 0;
  for (int i = 0; i < reglas.numero_de_reglas(); ++i) {
    if (reglas.modificada(i)) {
      poblacion.poner_codigo(poblacion.numero_de_individuos() - 1, reglas.codigo(i));
      peso_del_individuo(poblacion, ejemplos, tabla, poblacion.numero_de_individuos() - 1,
                         aciertos_de_la_regla, fallos_de_la_regla);
      reglas.sumar_ejemplos(i, aciertos_de_la_regla, fallos_de_la_regla);
      reglas.marcar_modificada(i, false);
    }
  }
}

std::vector<Regla> aprender(const difuso::Variables& variables, const datos::Ejemplos& ejemplos,
                            const Opciones& opciones) {
  // El informe (el antiguo <tarea>.res) solo se escribe si se pide.
  const bool informe = !opciones.informe.empty();
  const std::string fichero = opciones.informe.string();

  variables.pintar();

  if (informe) {
    std::ofstream out(fichero);
    out << "Experimento: " << fichero << '\n';
    out << std::format("{:%F %T} UTC\n", std::chrono::time_point_cast<std::chrono::seconds>(
                                             std::chrono::system_clock::now()));
    out << "===================================================\n\n";
  }

  GeneradorDeAzar generador_propio;
  GeneradorDeAzar& azar = opciones.azar != nullptr ? *opciones.azar : generador_propio;
  azar.sembrar(opciones.semilla);
  comun::progreso(Verbosidad::depuracion) << "Semilla " << opciones.semilla << '\n';

  const int tama_valor = variables.etiquetas_de_atributos();
  const int n_clases = variables.numero_de_etiquetas(variables.consecuente());
  // El nivel de variable: el de cada atributo y el umbral
  const int tama_nivel = variables.numero_de_atributos() + 1;

  comun::progreso(Verbosidad::detalle) << "Calculando medidas de informacion\n";
  const std::vector<std::vector<double>> medidas = medidas_de_informacion(
      variables, ejemplos.numero_de_ejemplos() > 10000 ? ejemplos.primeros(1000) : ejemplos);
  comun::progreso(Verbosidad::detalle) << "Terminado el calculo de las medidas de informacion\n";

  reglas::ConjuntoDeReglas nuevas_descriptivas;
  nuevas_descriptivas.poner_variables(variables);
  reglas::ConjuntoDeReglas nuevas_discriminantes = nuevas_descriptivas;
  reglas::ConjuntoDeReglas descriptivas = nuevas_descriptivas;
  reglas::ConjuntoDeReglas discriminantes = nuevas_descriptivas;
  reglas::ConjuntoDeReglas nuevos_patrones = nuevas_descriptivas;
  reglas::ConjuntoDeReglas patrones = nuevas_descriptivas;

  genetico::MultiPoblacion poblacion(n_clases, n_clases * 10, tama_valor, tama_nivel,
                                     2.0 / tama_valor, 0, 0.01);

  const int total_ejemplos = ejemplos.numero_de_ejemplos();
  const int tamano_episodio = static_cast<int>(opciones.tamano_episodio);
  // Para la línea de cada episodio (Verbosidad::episodios).
  const int total_episodios = (total_ejemplos + tamano_episodio - 1) / tamano_episodio;
  // En cuántos episodios ha aprendido reglas, más uno; y el episodio actual (que empieza en
  // indice_episodio * tamano_episodio).
  int episodios_con_reglas = 1;
  int indice_episodio = 0;
  const double margen = 0.10;
  double umbral_acierto = 0.7;
  double acumulado = 0.0;
  double acierto_actual = 0;
  double acierto_anterior = 0;
  int inicio = 0;
  bool salir = false;
  const std::clock_t empieza_el_reloj = std::clock();
  datos::Ejemplos ejemplos_del_episodio;

  while (!salir) {
    int fin = inicio + tamano_episodio;

    if (fin >= total_ejemplos) {
      fin = total_ejemplos;
      salir = true;
    }

    ejemplos_del_episodio = ejemplos.subconjunto(inicio, fin - 1);

    comun::progreso(Verbosidad::detalle)
        << "\n=================================================== Episodio de " << inicio << " a "
        << fin << " ====================================\n";
    const int episodio = indice_episodio + 1;
    const int primer_ejemplo = inicio;
    const int ultimo_ejemplo = fin - 1;
    const int reglas_al_empezar = descriptivas.numero_de_reglas();
    const auto empieza_el_episodio = std::chrono::steady_clock::now();

    const double consumido = static_cast<double>(std::clock() - empieza_el_reloj) / CLOCKS_PER_SEC;
    comun::progreso(Verbosidad::detalle)
        << "===================== Tiempo Consumido: " << consumido << "      ETA: "
        << (consumido / (indice_episodio + 1)) * (total_episodios - indice_episodio) << '\n';
    comun::progreso(Verbosidad::detalle)
        << "=================================================== Tamano "
        << ejemplos_del_episodio.numero_de_ejemplos()
        << " ====================================\n\n";
    comun::progreso(Verbosidad::depuracion) << "[Paso: " << tamano_episodio << "]\n";
    inicio = fin;

    comun::progreso(Verbosidad::depuracion) << "Creando la tabla de adaptaciones.....\n";
    const TablaDeAdaptaciones tabla(variables, ejemplos_del_episodio);
    comun::progreso(Verbosidad::depuracion) << "Tabla de adaptaciones creada!\n";

    comun::progreso(Verbosidad::detalle)
        << "  Numero de Reglas: " << descriptivas.numero_de_reglas() << "("
        << descriptivas.numero_con_peso(0.5) << ")\n";

    const double ultimo_acierto = acierto_actual;
    acumulado += ultimo_acierto;
    if (acumulado == 0) {
      umbral_acierto = 1;
    }
    comun::progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el anterior conjunto de ejemplos : " << ultimo_acierto
        << '\n';

    acierto_actual = aciertos(descriptivas, variables, ejemplos_del_episodio);
    comun::progreso(Verbosidad::detalle)
        << "Aciertos de la descriptiva sobre el nuevo conjunto de ejemplos : " << acierto_actual
        << '\n';
    const double acierto_al_empezar = acierto_actual;

    // El umbral sale del acierto sobre un episodio anterior: el de hace 10 cada 10 episodios y el
    // primero hasta el décimo. Los dos primeros episodios, sin anterior que mirar, 1.
    if (indice_episodio >= 10 && indice_episodio % 10 == 0) {
      datos::Ejemplos anteriores = ejemplos.subconjunto(
          (indice_episodio - 10) * tamano_episodio, (indice_episodio - 9) * tamano_episodio - 1);
      const double acierto = aciertos(descriptivas, variables, anteriores);
      umbral_acierto = acierto - margen * acierto;
    } else if (indice_episodio < 10 && indice_episodio > 1) {
      datos::Ejemplos anteriores = ejemplos.subconjunto(0, tamano_episodio - 1);
      const double acierto = aciertos(descriptivas, variables, anteriores);
      umbral_acierto = acierto - margen * acierto;
    } else if (indice_episodio < 10) {
      umbral_acierto = 1;
    }

    ++indice_episodio;

    comun::progreso(Verbosidad::detalle)
        << "Diferencia de acierto entre los dos últimos episodios: "
        << acierto_anterior - ultimo_acierto << '\n';
    acierto_anterior = ultimo_acierto;

    comun::progreso(Verbosidad::detalle)
        << "En la iteración " << episodios_con_reglas << " el umbral se ha fijado a "
        << umbral_acierto << " (" << umbral_acierto * (1 + margen) << ")\n";

    descriptivas.marcar_todas_modificadas();
    discriminantes.marcar_todas_modificadas();

    actualizar_pesos(poblacion, ejemplos_del_episodio, descriptivas, tabla);
    actualizar_pesos(poblacion, ejemplos_del_episodio, discriminantes, tabla);

    discriminantes.recalcular_peso();
    descriptivas.recalcular_peso();

    acierto_actual = aciertos(descriptivas, variables, ejemplos_del_episodio);
    comun::progreso(Verbosidad::detalle)
        << "Aciertos despues de ajustar los pesos de las reglas: " << acierto_actual << '\n';
    const double acierto_tras_ajustar_pesos = acierto_actual;

    if (acierto_actual < umbral_acierto) {
      datos::Ejemplos ejemplos_a_cubrir = ejemplos_del_episodio;

      for (const int regla : reglas_que_disparan(descriptivas, ejemplos_a_cubrir)) {
        nuevas_descriptivas.anadir(descriptivas.codigo(regla), descriptivas.aciertos(regla),
                                   descriptivas.fallos(regla), false);

        const double aciertos_de_la_regla = discriminantes.aciertos(regla);
        const double fallos_de_la_regla = discriminantes.fallos(regla);
        nuevas_discriminantes.anadir(discriminantes.codigo(regla), aciertos_de_la_regla,
                                     fallos_de_la_regla, false);
        nuevos_patrones.anadir(patrones.codigo(regla), aciertos_de_la_regla, fallos_de_la_regla,
                               false);
      }

      comun::progreso(Verbosidad::detalle)
          << "Reglas no filtradas: " << nuevas_descriptivas.numero_de_reglas() << "("
          << nuevas_descriptivas.numero_con_peso(0.5) << ")\n";

      const TablaDeAdaptaciones tabla_a_cubrir(variables, ejemplos_a_cubrir);

      comun::progreso(Verbosidad::detalle)
          << "\n-------------------------------------------> Nuevo numero de ejemplos: "
          << ejemplos_a_cubrir.numero_de_ejemplos() << '\n';

      acierto_actual = aciertos(nuevas_descriptivas, variables, ejemplos_a_cubrir);
      comun::progreso(Verbosidad::detalle)
          << "Aciertos de la descriptiva antes de lanzar el genetico: " << acierto_actual << '\n';

      const int reglas_iniciales = nuevas_descriptivas.numero_de_reglas();
      std::string ultimo_patron;
      bool primera_vez = true;
      bool reducir_reglas = false;
      bool anadida = false;
      std::vector<int> reglas_a_eliminar;

      do {
        const int sin_clasificar_antes = ejemplos_a_cubrir.sin_clasificar_bien();
        const std::vector<int> ejemplos_por_clase =
            ejemplos_a_cubrir.sin_clasificar_bien_por_clase(variables.consecuente(), n_clases);

        std::vector<int> basica_para_regla(indice(nuevas_descriptivas.numero_de_reglas() + 1));
        std::vector<int> errores_para_regla(indice(nuevas_descriptivas.numero_de_reglas() + 1));
        contar_basicos_y_errores(ejemplos_a_cubrir, basica_para_regla, errores_para_regla);

        const Evaluacion evaluacion{poblacion,           variables,         ejemplos_a_cubrir,
                                    nuevas_descriptivas, reducir_reglas,    ejemplos_por_clase,
                                    tabla_a_cubrir,      basica_para_regla, errores_para_regla,
                                    reglas_iniciales};
        buscar_regla(evaluacion, azar, nuevas_descriptivas, nuevas_discriminantes, nuevos_patrones,
                     medidas, primera_vez, reglas_a_eliminar, anadida);

        primera_vez = false;

        aciertos(nuevas_descriptivas, variables, ejemplos_a_cubrir);
        const std::string patron_actual =
            patron_de_regla(nuevos_patrones, nuevos_patrones.numero_de_reglas() - 1);
        if (patron_actual == ultimo_patron) {
          anadida = false;
        }
        ultimo_patron = patron_actual;

        if (anadida && !reglas_a_eliminar.empty()) {
          quitar_reglas(nuevas_descriptivas, nuevas_discriminantes, nuevos_patrones,
                        reglas_a_eliminar);
          aciertos(nuevas_descriptivas, variables, ejemplos_a_cubrir);
        }

        if (poblacion.valoracion(0)[0] > 0 &&
            sin_clasificar_antes == ejemplos_a_cubrir.sin_clasificar_bien()) {
          comun::progreso(Verbosidad::detalle)
              << "+++++++++++++++++++++ ERROR ++++++++++++++++++++++\n";
        }

        if (poblacion.valoracion(0)[0] != -2 * ejemplos_a_cubrir.numero_de_ejemplos() &&
            !reducir_reglas && !anadida) {
          reducir_reglas = true;
          anadida = true;
        }
      } while (anadida);

      ++episodios_con_reglas;

      if (indice_episodio > 1) {
        const datos::Ejemplos anteriores = ejemplos.subconjunto(
            indice_episodio > 10 ? (indice_episodio - 10) * tamano_episodio : 0, inicio - 1);
        const TablaDeAdaptaciones tabla_anteriores(variables, anteriores);
        actualizar_pesos(poblacion, anteriores, nuevas_descriptivas, tabla_anteriores);
        actualizar_pesos(poblacion, anteriores, nuevas_discriminantes, tabla_anteriores);
      }

      descriptivas.anadir_al_principio(nuevas_descriptivas);
      discriminantes.anadir_al_principio(nuevas_discriminantes);
      patrones.anadir_al_principio(nuevos_patrones);
      comun::progreso(Verbosidad::detalle)
          << "Aniadidas " << nuevas_descriptivas.numero_de_reglas() - reglas_iniciales
          << " reglas\n";

      acierto_actual = aciertos(descriptivas, variables, ejemplos_del_episodio);
      comun::progreso(Verbosidad::detalle)
          << "Acierto antes de analizar patrones: " << acierto_actual << '\n';

      if (acierto_tras_ajustar_pesos < acierto_actual) {
        comun::progreso(Verbosidad::detalle) << "............................. Analizando patrones "
                                                "entre reglas .......................\n";
        combinar_reglas(variables, discriminantes, descriptivas, patrones,
                        nuevas_descriptivas.numero_de_reglas());
        acierto_actual = aciertos(descriptivas, variables, ejemplos_del_episodio);
        comun::progreso(Verbosidad::detalle)
            << "Acierto despues de analizar patrones: " << acierto_actual << '\n';
      } else {
        for (int i = 0; i < nuevas_descriptivas.numero_de_reglas(); ++i) {
          descriptivas.quitar(0);
          discriminantes.quitar(0);
          patrones.quitar(0);
        }
        acierto_actual = aciertos(descriptivas, variables, ejemplos_del_episodio);
      }
    }

    // Una línea por episodio. Solo con lo que ya se ha calculado: aciertos marca los ejemplos y
    // llamarlo aquí cambiaría las reglas.
    if (comun::se_escribe(Verbosidad::episodios)) {
      const std::chrono::duration<double> segundos =
          std::chrono::steady_clock::now() - empieza_el_episodio;
      comun::progreso(Verbosidad::episodios) << std::format(
          "INSLV: episodio {} de {} (ejemplos {} a {}): reglas {} -> {}, acierto {:.3f} -> "
          "{:.3f}, {:.2f} s\n",
          episodio, total_episodios, primer_ejemplo, ultimo_ejemplo, reglas_al_empezar,
          descriptivas.numero_de_reglas(), acierto_al_empezar, acierto_actual, segundos.count());
    }
    if (informe) {
      std::ofstream salida(fichero, std::ios::app);
      salida << "*****************************************************************\n";
      for (int i = 0; i < nuevas_descriptivas.numero_de_reglas(); ++i) {
        salida << i << ") " << nuevas_descriptivas.texto(i) << '\n';
      }
      salida << '\n';
    }

    while (nuevas_descriptivas.numero_de_reglas() > 0) {
      nuevas_descriptivas.quitar_ultima();
      nuevas_discriminantes.quitar_ultima();
      nuevos_patrones.quitar_ultima();
    }
  }

  if (informe) {
    escribir_informe(fichero, variables, ejemplos_del_episodio, descriptivas, discriminantes,
                     patrones,
                     static_cast<double>(std::clock() - empieza_el_reloj) / CLOCKS_PER_SEC);
  }

  // Las descriptivas (descriptivas) o sus versiones discriminantes (discriminantes), emparejadas
  // regla a regla.
  const reglas::ConjuntoDeReglas& elegidas =
      opciones.reglas == TipoDeReglas::discriminantes ? discriminantes : descriptivas;
  std::vector<Regla> reglas;
  for (int i = 0; i < elegidas.numero_de_reglas(); ++i) {
    Regla regla;
    regla.antecedente = elegidas.antecedente(i);
    regla.consecuente = indice(elegidas.clase(i));
    regla.cubiertos = elegidas.aciertos(i);
    regla.peso = elegidas.peso(i);
    reglas.push_back(regla);
  }
  return reglas;
}

}  // namespace inslv::aprendizaje
