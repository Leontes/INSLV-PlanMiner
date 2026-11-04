// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/tabla.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

TablaDeAdaptaciones::TablaDeAdaptaciones(const difuso::Variables& variables,
                                         const datos::Ejemplos& ejemplos)
    : n_antecedente_(variables.numero_de_atributos()), consecuente_(variables.consecuente()) {
  const int n = variables.numero_de_variables();
  const int m = ejemplos.numero_de_ejemplos();

  adaptaciones_.resize(indice(n));
  for (int v = 0; v < n; ++v) {
    adaptaciones_[indice(v)].assign(indice(variables.numero_de_etiquetas(v)),
                                    std::vector<double>(indice(m)));
  }

  for (int e = 0; e < m; ++e) {
    for (int v = 0; v < n; ++v) {
      const double valor = ejemplos.valor(e, v);
      std::vector<std::vector<double>>& etiquetas = adaptaciones_[indice(v)];
      int valores_distinto_cero = 0;
      for (std::size_t j = 0; j < etiquetas.size(); ++j) {
        const double adaptacion = variables.adaptacion(valor, v, static_cast<int>(j));
        etiquetas[j][indice(e)] = adaptacion;
        if (adaptacion > 0 && adaptacion <= 1) {
          ++valores_distinto_cero;
        }
      }
      if (valores_distinto_cero == 0) {
        throw std::invalid_argument("inslv: el valor " + std::to_string(valor) +
                                    " de la variable " + std::to_string(v) +
                                    " no se adapta a ninguna de sus etiquetas");
      }
    }
  }
}

double TablaDeAdaptaciones::adaptacion_a_etiqueta(int ejemplo, int variable, int etiqueta) const {
  return adaptaciones_[indice(variable)][indice(etiqueta)][indice(ejemplo)];
}

double TablaDeAdaptaciones::adaptacion_a_variable(int ejemplo, int variable,
                                                  const char* etiquetas) const {
  const std::vector<std::vector<double>>& tabla = adaptaciones_[indice(variable)];
  double max = 0;
  for (std::size_t etiqueta = 0; etiqueta < tabla.size() && max < 1; ++etiqueta) {
    if (etiquetas[etiqueta] == '1') {
      const double adaptacion = tabla[etiqueta][indice(ejemplo)];
      if (max == 0) {
        max = adaptacion;
      } else if (adaptacion > 0) {
        max = 1;
      }
    }
  }
  return max;
}

double TablaDeAdaptaciones::adaptacion_al_antecedente(int ejemplo, const std::string& etiquetas,
                                                      const std::vector<double>& niveles) const {
  double max = 1;
  std::size_t trozo = 0;
  for (int v = 0; v < n_antecedente_ && max > 0; ++v) {
    if (niveles[indice(v)] >= niveles.back()) {
      const double aux = adaptacion_a_variable(ejemplo, v, etiquetas.data() + trozo);
      if (aux < max) {
        max = aux;
      }
    }
    trozo += adaptaciones_[indice(v)].size();
  }
  return max;
}

void TablaDeAdaptaciones::consecuente(int ejemplo, int etiqueta, double& pos, double& neg) const {
  pos = adaptacion_a_etiqueta(ejemplo, consecuente_, etiqueta);
  neg = pos == 1 ? 0 : 1;
}

int TablaDeAdaptaciones::mejor_etiqueta(int ejemplo, int variable) const {
  const std::vector<std::vector<double>>& tabla = adaptaciones_[indice(variable)];
  std::size_t mejor = 0;
  double max = 0;
  for (std::size_t etiqueta = 0; etiqueta < tabla.size() && max < 1; ++etiqueta) {
    if (tabla[etiqueta][indice(ejemplo)] > max) {
      max = tabla[etiqueta][indice(ejemplo)];
      mejor = etiqueta;
    }
  }
  return static_cast<int>(mejor);
}

}  // namespace inslv::aprendizaje
