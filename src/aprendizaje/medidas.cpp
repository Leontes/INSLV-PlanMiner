// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/medidas.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Las probabilidades de cada etiqueta de la variable en los ejemplos sin clasificar bien.
std::vector<double> probabilidad_de_cada_etiqueta(const difuso::Variables& variables,
                                                  const datos::Ejemplos& ejemplos, int variable) {
  const auto n_casos = indice(variables.numero_de_etiquetas(variable));
  std::vector<double> p(n_casos, 0);
  std::vector<double> termino(n_casos);
  int ne = 0;
  for (int i = 0; i < ejemplos.numero_de_ejemplos(); ++i) {
    if (!ejemplos.bien_clasificado(i)) {
      double suma = 0;
      for (std::size_t j = 0; j < n_casos; ++j) {
        termino[j] =
            variables.adaptacion(ejemplos.valor(i, variable), variable, static_cast<int>(j));
        suma += termino[j];
      }
      // Añadir los valores a la sumatoria
      if (suma > 0) {
        for (std::size_t j = 0; j < n_casos; ++j) {
          p[j] = p[j] + (termino[j] / suma);
        }
        ++ne;
      }
    }
  }
  if (ne > 0) {
    comun::progreso(Verbosidad::depuracion) << "NE: " << ne << '\n';
    for (double& probabilidad : p) {
      probabilidad = probabilidad / ne;
    }
  }
  return p;
}

// Las probabilidades de cada par de etiquetas de dos variables, igual.
std::vector<std::vector<double>> probabilidad_conjunta(const difuso::Variables& variables,
                                                       const datos::Ejemplos& ejemplos, int var1,
                                                       int var2) {
  const auto n_casos1 = indice(variables.numero_de_etiquetas(var1));
  const auto n_casos2 = indice(variables.numero_de_etiquetas(var2));
  std::vector<std::vector<double>> m(n_casos1, std::vector<double>(n_casos2, 0));
  std::vector<std::vector<double>> termino(n_casos1, std::vector<double>(n_casos2));
  int ne = 0;
  for (int i = 0; i < ejemplos.numero_de_ejemplos(); ++i) {
    if (!ejemplos.bien_clasificado(i)) {
      double suma = 0;
      for (std::size_t j = 0; j < n_casos1; ++j) {
        for (std::size_t k = 0; k < n_casos2; ++k) {
          termino[j][k] = variables.adaptacion(ejemplos.valor(i, var1), var1, static_cast<int>(j)) *
                          variables.adaptacion(ejemplos.valor(i, var2), var2, static_cast<int>(k));
          suma = suma + termino[j][k];
        }
      }
      if (suma > 0) {
        for (std::size_t j = 0; j < n_casos1; ++j) {
          for (std::size_t k = 0; k < n_casos2; ++k) {
            m[j][k] = m[j][k] + (termino[j][k] / suma);
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

}  // namespace

std::vector<std::vector<double>> medidas_de_informacion(const difuso::Variables& variables,
                                                        const datos::Ejemplos& ejemplos) {
  const int clase = variables.consecuente();
  const auto n_clases = indice(variables.numero_de_etiquetas(clase));
  std::vector<std::vector<double>> medidas(indice(variables.numero_de_atributos()),
                                           std::vector<double>(n_clases + 1));

  const std::vector<double> p_clase = probabilidad_de_cada_etiqueta(variables, ejemplos, clase);

  for (std::size_t i = 0; i < medidas.size(); ++i) {
    const int atributo = static_cast<int>(i);
    const std::vector<double> p_atributo =
        probabilidad_de_cada_etiqueta(variables, ejemplos, atributo);
    const std::vector<std::vector<double>> p_conjunta =
        probabilidad_conjunta(variables, ejemplos, atributo, clase);

    double informacion = 0;
    double entropia = 0;
    for (std::size_t k = 0; k < n_clases; ++k) {
      double informacion_clase = 0;
      double entropia_clase = 0;
      for (std::size_t q = 0; q < p_atributo.size(); ++q) {
        double termino =
            p_conjunta[q][k] == 0
                ? 0
                : p_conjunta[q][k] * std::log((p_atributo[q] * p_clase[k]) / p_conjunta[q][k]);
        informacion_clase = informacion_clase - termino;
        informacion = informacion - termino;

        termino = p_conjunta[q][k] == 0 ? 0 : p_conjunta[q][k] * std::log(p_conjunta[q][k]);
        entropia_clase = entropia_clase - termino;
        entropia = entropia - termino;
      }
      medidas[i][k + 1] = entropia_clase == 0 ? 0 : informacion_clase / entropia_clase;
    }
    medidas[i][0] = entropia == 0 ? 0 : informacion / entropia;

    variables.pintar_nombre(atributo);
    comun::progreso(Verbosidad::depuracion)
        << ": " << informacion << "/" << entropia << ": " << medidas[i][0] << '\n';
  }
  return medidas;
}

}  // namespace inslv::aprendizaje
