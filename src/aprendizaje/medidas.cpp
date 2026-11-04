// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/medidas.h"

#include <cmath>
#include <cstddef>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Las probabilidades de cada etiqueta de la variable en los ejemplos sin clasificar bien.
std::vector<double> CalcularProbabilidadSimpleVariable(const difuso::Variables& V,
                                                       const datos::Ejemplos& E, int variable) {
  const auto n_casos = indice(V.numero_de_etiquetas(variable));
  std::vector<double> p(n_casos, 0);
  std::vector<double> aux(n_casos);
  int ne = 0;
  for (int i = 0; i < E.numero_de_ejemplos(); ++i) {
    if (!E.bien_clasificado(i)) {
      double suma = 0;
      for (std::size_t j = 0; j < n_casos; ++j) {
        aux[j] = V.adaptacion(E.valor(i, variable), variable, static_cast<int>(j));
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
    comun::progreso(Verbosidad::depuracion) << "NE: " << ne << '\n';
    for (double& probabilidad : p) {
      probabilidad = probabilidad / ne;
    }
  }
  return p;
}

// Las probabilidades de cada par de etiquetas de dos variables, igual.
std::vector<std::vector<double>> CalcularProbabilidadConjunta2Variables(const difuso::Variables& V,
                                                                        const datos::Ejemplos& E,
                                                                        int var1, int var2) {
  const auto n_casos1 = indice(V.numero_de_etiquetas(var1));
  const auto n_casos2 = indice(V.numero_de_etiquetas(var2));
  std::vector<std::vector<double>> m(n_casos1, std::vector<double>(n_casos2, 0));
  std::vector<std::vector<double>> aux(n_casos1, std::vector<double>(n_casos2));
  int ne = 0;
  for (int i = 0; i < E.numero_de_ejemplos(); ++i) {
    if (!E.bien_clasificado(i)) {
      double suma = 0;
      for (std::size_t j = 0; j < n_casos1; ++j) {
        for (std::size_t k = 0; k < n_casos2; ++k) {
          aux[j][k] = V.adaptacion(E.valor(i, var1), var1, static_cast<int>(j)) *
                      V.adaptacion(E.valor(i, var2), var2, static_cast<int>(k));
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

}  // namespace

std::vector<std::vector<double>> MedidasDeInformacion(const difuso::Variables& V,
                                                      const datos::Ejemplos& E) {
  const int conse = V.consecuente();
  const auto n_clases = indice(V.numero_de_etiquetas(conse));
  std::vector<std::vector<double>> I(indice(V.numero_de_atributos()),
                                     std::vector<double>(n_clases + 1));

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

    V.pintar_nombre(atributo);
    comun::progreso(Verbosidad::depuracion) << ": " << I1 << "/" << H1 << ": " << I[i][0] << '\n';
  }
  return I;
}

}  // namespace inslv
