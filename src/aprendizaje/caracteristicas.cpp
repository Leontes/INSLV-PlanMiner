// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/caracteristicas.hpp"

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

std::vector<Condicion> caracterizar(const reglas::ConjuntoDeReglas& reglas, int regla,
                                    const difuso::Variables& variables,
                                    const datos::Ejemplos& ejemplos, double tolerancia) {
  const genetico::Codigo& codigo = reglas.codigo(regla);
  const int clase = reglas.clase(regla);
  const int consecuente = variables.consecuente();

  std::vector<datos::Ejemplo> cubiertos;
  for (int e = 0; e < ejemplos.numero_de_ejemplos(); ++e) {
    if (ejemplos.clase(consecuente, e) == clase) {
      datos::Ejemplo ejemplo = ejemplos.ejemplo(e);
      if (variables.adaptacion(ejemplo, codigo.etiquetas, codigo.niveles) > 0) {
        cubiertos.push_back(std::move(ejemplo));
      }
    }
  }
  if (cubiertos.empty()) {
    return reglas.antecedente(regla);
  }
  const double necesarios = (1 - tolerancia) * static_cast<double>(cubiertos.size());

  std::vector<Condicion> antecedente = reglas.antecedente(regla);
  std::vector<Condicion> condiciones;
  for (int atributo = 0; atributo < variables.numero_de_atributos(); ++atributo) {
    const std::size_t n_etiquetas = indice(variables.numero_de_etiquetas(atributo));
    // Las etiquetas que admite la regla: las de su condición o, si no tiene, todas.
    std::vector<std::size_t> admitidas(n_etiquetas);
    std::iota(admitidas.begin(), admitidas.end(), std::size_t{0});
    const auto propia =
        std::find_if(antecedente.begin(), antecedente.end(),
                     [&](const Condicion& c) { return c.atributo == indice(atributo); });
    if (propia != antecedente.end()) {
      admitidas = propia->etiquetas;
    }

    // Qué ejemplos se adaptan a cada etiqueta.
    std::vector<std::vector<bool>> se_adaptan(n_etiquetas, std::vector<bool>(cubiertos.size()));
    std::vector<std::size_t> cuantos(n_etiquetas, 0);
    for (const std::size_t etiqueta : admitidas) {
      for (std::size_t e = 0; e < cubiertos.size(); ++e) {
        se_adaptan[etiqueta][e] = variables.adaptacion(cubiertos[e].valor(atributo), atributo,
                                                       static_cast<int>(etiqueta)) > 0;
        if (se_adaptan[etiqueta][e]) {
          ++cuantos[etiqueta];
        }
      }
    }
    std::stable_sort(admitidas.begin(), admitidas.end(),
                     [&](std::size_t a, std::size_t b) { return cuantos[a] > cuantos[b]; });

    // Las de más ejemplos, hasta cubrir los necesarios.
    std::vector<std::size_t> elegidas;
    std::vector<bool> cubierto(cubiertos.size(), false);
    std::size_t total = 0;
    for (const std::size_t etiqueta : admitidas) {
      if (static_cast<double>(total) >= necesarios) {
        break;
      }
      elegidas.push_back(etiqueta);
      for (std::size_t e = 0; e < cubiertos.size(); ++e) {
        if (se_adaptan[etiqueta][e] && !cubierto[e]) {
          cubierto[e] = true;
          ++total;
        }
      }
    }
    if (elegidas.size() < n_etiquetas) {
      std::sort(elegidas.begin(), elegidas.end());
      condiciones.push_back({indice(atributo), elegidas});
    }
  }
  return condiciones;
}

}  // namespace inslv::aprendizaje
