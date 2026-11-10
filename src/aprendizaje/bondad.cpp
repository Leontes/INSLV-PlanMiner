// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/bondad.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Si la valoración v2 es mejor que la v en sus tres primeros valores.
bool mejor_valoracion(const genetico::Valoracion& v2, const genetico::Valoracion& v) {
  return (v2[0] > v[0]) || (v2[0] == v[0] && v2[1] > v[1]) ||
         (v2[0] == v[0] && v2[1] == v[1] && v2[2] > v[2]);
}

// Igual, o si es igual de buena.
bool mejor_o_igual_valoracion(const genetico::Valoracion& v2, const genetico::Valoracion& v) {
  return (v2[0] > v[0]) || (v2[0] == v[0] && v2[1] > v[1]) ||
         (v2[0] == v[0] && v2[1] == v[1] && v2[2] >= v[2]);
}

// Si s tiene algún cero: si no, el atributo admite todas sus etiquetas y es irrelevante.
bool es_relevante(std::string_view s) {
  return !std::all_of(s.begin(), s.end(), [](char bit) { return bit == '1'; });
}

}  // namespace

void valorar_modificados(const Evaluacion& evaluacion) {
  genetico::MultiPoblacion& poblacion = evaluacion.poblacion;
  // Las reglas que eliminaría cada individuo solo interesan al evaluar la regla que se añade.
  std::vector<int> reglas_a_eliminar;
  genetico::Valoracion valor{};
  for (int i = 0; i < poblacion.numero_de_individuos(); ++i) {
    if (poblacion.modificado(i)) {
      bondad(evaluacion, i, valor, reglas_a_eliminar);
      poblacion.valorar(i, valor);
    }
  }
}

void generalizar(const Evaluacion& evaluacion, int individuo, std::vector<int>& reglas_a_eliminar) {
  genetico::MultiPoblacion& poblacion = evaluacion.poblacion;
  const difuso::Variables& variables = evaluacion.variables;
  std::string& etiquetas = poblacion.etiquetas(individuo);
  std::vector<double>& niveles = poblacion.niveles(individuo);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  std::size_t tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto longitud = indice(variables.numero_de_etiquetas(static_cast<int>(j)));
    if (niveles[j] < umbral) {
      etiquetas.replace(tama, longitud, longitud, '1');
    }
    tama += longitud;
  }

  genetico::Valoracion valor{};
  genetico::Valoracion valor2{};
  bondad(evaluacion, individuo, valor, reglas_a_eliminar);
  poblacion.valorar(individuo, valor);

  // Quita cada atributo relevante si la regla mejora sin él.
  for (std::size_t j = 0; j < atributos; ++j) {
    if (niveles[j] >= umbral) {
      const double aux = niveles[j];
      niveles[j] = umbral - 0.001;

      bondad(evaluacion, individuo, valor2, reglas_a_eliminar);

      if (mejor_valoracion(valor2, valor)) {
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
    const auto longitud = indice(variables.numero_de_etiquetas(static_cast<int>(j)));

    if (niveles[j] >= umbral) {
      for (std::size_t s = 0; s < longitud; ++s) {
        if (etiquetas[tama + s] == '0') {
          etiquetas[tama + s] = '1';
          bondad(evaluacion, individuo, valor2, reglas_a_eliminar);
          if (mejor_o_igual_valoracion(valor2, valor)) {
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

  bondad(evaluacion, individuo, valor, reglas_a_eliminar);
  poblacion.valorar(individuo, valor);
}

void especializar(const Evaluacion& evaluacion, int individuo,
                  std::vector<int>& reglas_a_eliminar) {
  genetico::MultiPoblacion& poblacion = evaluacion.poblacion;
  const difuso::Variables& variables = evaluacion.variables;
  std::string& etiquetas = poblacion.etiquetas(individuo);
  std::vector<double>& niveles = poblacion.niveles(individuo);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  genetico::Valoracion valor{};
  genetico::Valoracion valor2{};
  bondad(evaluacion, individuo, valor, reglas_a_eliminar);
  poblacion.valorar(individuo, valor);

  std::size_t tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto longitud = indice(variables.numero_de_etiquetas(static_cast<int>(j)));

    if (niveles[j] < umbral) {
      // Añade el atributo a la regla si, quitándole alguna etiqueta, no empeora.
      const double aux = niveles[j];
      bool mejora = false;
      niveles[j] = umbral;
      etiquetas.replace(tama, longitud, longitud, '1');

      for (std::size_t s = 0; s < longitud; ++s) {
        etiquetas[tama + s] = '0';
        bondad(evaluacion, individuo, valor2, reglas_a_eliminar);

        if (mejor_o_igual_valoracion(valor2, valor)) {
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
          bondad(evaluacion, individuo, valor2, reglas_a_eliminar);
          if (mejor_valoracion(valor2, valor)) {
            valor = valor2;
          } else {
            etiquetas[tama + s] = '1';
          }
        }
      }
    }

    tama += longitud;
  }

  bondad(evaluacion, individuo, valor, reglas_a_eliminar);
  poblacion.valorar(individuo, valor);
}

double peso_del_individuo(const genetico::MultiPoblacion& poblacion,
                          const TablaDeAdaptaciones& tabla, int individuo, double& aciertos,
                          double& fallos) {
  const std::string& etiquetas = poblacion.etiquetas(individuo);
  const std::vector<double>& niveles = poblacion.niveles(individuo);
  const int clase = poblacion.clase(individuo);

  std::vector<double> adaptacion;
  tabla.adaptaciones_al_antecedente(etiquetas, niveles, adaptacion);
  const std::vector<double>& de_la_clase = tabla.adaptaciones_a_clase(clase);

  double positivos = 0;
  double negativos = 0;
  for (std::size_t j = 0; j < adaptacion.size(); ++j) {
    const double aux_p = de_la_clase[j];
    const double aux_n = aux_p == 1 ? 0 : 1;
    positivos = positivos + (adaptacion[j] * aux_p);
    negativos = negativos + (adaptacion[j] * aux_n);
  }

  aciertos = positivos;
  fallos = negativos;

  return (positivos + 1) / (positivos + negativos + 1);
}

double bondad(const Evaluacion& evaluacion, int individuo, genetico::Valoracion& valor,
              std::vector<int>& reglas_a_eliminar) {
  genetico::MultiPoblacion& poblacion = evaluacion.poblacion;
  const difuso::Variables& variables = evaluacion.variables;
  const datos::Ejemplos& ejemplos = evaluacion.ejemplos;
  const reglas::ConjuntoDeReglas& reglas = evaluacion.reglas;
  const std::vector<int>& sin_clasificar_por_clase = evaluacion.sin_clasificar_por_clase;

  const int ne = ejemplos.numero_de_ejemplos();
  const int n_reglas = reglas.numero_de_reglas();
  const int clase = poblacion.clase(individuo);
  const int sin_clasificar = sin_clasificar_por_clase[indice(clase)];

  const std::string& etiquetas = poblacion.etiquetas(individuo);
  std::vector<double>& niveles = poblacion.niveles(individuo);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  std::size_t variables_irrelevantes = 0;
  int ceros_variables = 0;
  std::size_t pos = 0;  // posicion en el cromosoma nivel de valor

  for (std::size_t j = 0; j < atributos; ++j) {
    const auto tamanio = indice(variables.numero_de_etiquetas(static_cast<int>(j)));
    const std::string_view trozo = std::string_view(etiquetas).substr(pos, tamanio);
    if (niveles[j] < umbral) {
      ++variables_irrelevantes;
    } else if (es_relevante(trozo)) {
      ceros_variables += static_cast<int>(std::count(trozo.begin(), trozo.end(), '0'));
    } else {
      niveles[j] = umbral - 0.001;
      ++variables_irrelevantes;
    }
    pos = pos + tamanio;
  }

  const int variables_relevantes = static_cast<int>(atributos - variables_irrelevantes);

  // comprobar que es una regla valida
  if (!variables.es_valida(etiquetas, niveles) || variables_irrelevantes >= atributos) {
    valor.fill(-2 * ne);
    return 0;
  }

  // Calculo previo para obtener el peso de la regla
  std::vector<double> adaptacion;
  evaluacion.tabla.adaptaciones_al_antecedente(etiquetas, niveles, adaptacion);
  const std::vector<double>& de_la_clase = evaluacion.tabla.adaptaciones_a_clase(clase);
  double positivos = 0;
  double negativos = 0;
  double afecta = 0;
  for (std::size_t j = 0; j < adaptacion.size(); ++j) {
    const double aux_p = de_la_clase[j];
    const double aux_n = aux_p == 1 ? 0 : 1;
    const double a = adaptacion[j];

    if (a * aux_p > 0) {
      ++afecta;
    }

    positivos = positivos + (a * aux_p);
    negativos = negativos + (a * aux_n);
    adaptacion[j] = aux_p < aux_n ? -a : a;
  }

  const double peso = (positivos + 1) / (positivos + negativos + 1);
  const bool menos_infinito = !(positivos > 0);

  if (sin_clasificar > 0) {
    afecta = afecta / sin_clasificar;
  }

  // Dado el peso de la regla obtener la bondad de la misma.
  std::vector<int> basica_esta_regla(indice(n_reglas));
  std::vector<int> errores_esta_regla(indice(n_reglas));
  const auto anota = [&](std::vector<int>& cuenta, int ejemplo, int aislada) {
    if (ejemplos.regla_aislada(ejemplo) == aislada && ejemplos.regla_usada(ejemplo) >= 0 &&
        ejemplos.regla_usada(ejemplo) < n_reglas) {
      ++cuenta[indice(ejemplos.regla_usada(ejemplo))];
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
      if (a * peso > ejemplos.grado_negativo(j) ||
          (a * peso == ejemplos.grado_negativo(j) && peso > ejemplos.peso_negativo(j))) {
        if (!ejemplos.bien_clasificado(j)) {
          positivos = positivos + (a * peso);
          ++aciertos;
          anota(errores_esta_regla, j, -1);
        } else {
          anota(basica_esta_regla, j, 1);
        }
      }
    } else if (a < 0) {
      // La regla gana a la mejor de la clase del ejemplo.
      if (-a * peso > ejemplos.grado_positivo(j) ||
          (-a * peso == ejemplos.grado_positivo(j) && peso > ejemplos.peso_positivo(j))) {
        negativos = negativos +
                    ((-a * peso) /
                     sin_clasificar_por_clase[indice(ejemplos.clase(variables.consecuente(), j))]);
        if (!ejemplos.bien_clasificado(j)) {
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
  for (int i = evaluacion.reglas_no_eliminables; i < n_reglas; ++i) {
    const std::size_t r = indice(i);
    const int ganancia = (basica_esta_regla[r] - evaluacion.basica_para_regla[r]) +
                         (evaluacion.errores_para_regla[r] - errores_esta_regla[r]);
    if (ganancia >= 0) {
      reglas_a_eliminar.push_back(i);
      ++reglas_eliminables;
      aciertos_adicionales += ganancia;
      ganancia_en_variables = reglas.atributos_sobre_el_umbral(i) - ganancia_en_variables;
      diferencia_peso -= reglas.peso(i);
    }
  }

  positivos = sin_clasificar > 0 ? positivos / sin_clasificar : 0;

  if (aciertos + aciertos_adicionales - fallos > 0) {
    if (sin_clasificar > 0) {
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
    } else if (evaluacion.reducir_reglas && !(reglas_eliminables >= (-valor[0]) + 2)) {
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

  // El desempate cuando la regla sustituiría a una sola. El original tenía otra rama para las
  // descriptivas, que nunca se usaba (con discriminante a false): n+ - n- frente a la regla que
  // sustituye y la mejor proporción frente a las de su clase. Probada en la fase B: no cambia el
  // acierto ni mejora los dominios de PlanMiner, y en algún caso el bucle que empareja la
  // descriptiva con la discriminante (buscar_regla) no termina.
  if (reglas_eliminables == 1 && (valor[0] == -ne || valor[0] == 0)) {
    valor[2] = aciertos_adicionales > 0 ? 0.1 + aciertos_adicionales : diferencia_peso;
    valor[3] = -ganancia_en_variables;
    valor[4] = 0;
    valor[5] = ceros_variables;
    // El original ponía aquí el peso de la regla 1 (usaba reglas_eliminables, que aquí es 1, como
    // índice), sin existir siempre. No decidía nada: dos individuos solo empatan hasta aquí si los
    // dos pasan por esta rama, y entonces leían el mismo valor. Con el índice bien,
    // reglas_a_eliminar[0], sí decide, pero no mejora: cambia 155 de 1000 experimentos y el acierto
    // no se distingue del azar (privado/resultados/valor6-2026-11-06.csv).
    valor[6] = 0;
  }

  return peso;
}

}  // namespace inslv::aprendizaje
