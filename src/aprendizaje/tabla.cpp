// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/tabla.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

TablaDeAdaptaciones::TablaDeAdaptaciones(const difuso::Variables& V, const datos::Ejemplos& E)
    : n_antecedente_(V.numero_de_atributos()), consecuente_(V.consecuente()) {
  const int n = V.numero_de_variables();
  const int m = E.numero_de_ejemplos();

  adaptaciones_.resize(indice(n));
  for (int v = 0; v < n; ++v) {
    adaptaciones_[indice(v)].assign(indice(V.numero_de_etiquetas(v)),
                                    std::vector<double>(indice(m)));
  }

  for (int e = 0; e < m; ++e) {
    for (int v = 0; v < n; ++v) {
      const double valor = E.valor(e, v);
      std::vector<std::vector<double>>& etiquetas = adaptaciones_[indice(v)];
      int valores_distinto_cero = 0;
      for (std::size_t j = 0; j < etiquetas.size(); ++j) {
        const double adaptacion = V.adaptacion(valor, v, static_cast<int>(j));
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

double TablaDeAdaptaciones::Etiqueta(int ejemplo, int variable, int etiqueta) const {
  return adaptaciones_[indice(variable)][indice(etiqueta)][indice(ejemplo)];
}

double TablaDeAdaptaciones::Variable(int ejemplo, int variable, const char* etiquetas) const {
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

double TablaDeAdaptaciones::Antecedente(int ejemplo, const std::string& etiquetas,
                                        const std::vector<double>& niveles) const {
  double max = 1;
  std::size_t trozo = 0;
  for (int v = 0; v < n_antecedente_ && max > 0; ++v) {
    if (niveles[indice(v)] >= niveles.back()) {
      const double aux = Variable(ejemplo, v, etiquetas.data() + trozo);
      if (aux < max) {
        max = aux;
      }
    }
    trozo += adaptaciones_[indice(v)].size();
  }
  return max;
}

void TablaDeAdaptaciones::Consecuente(int ejemplo, int etiqueta, double& pos, double& neg) const {
  pos = Etiqueta(ejemplo, consecuente_, etiqueta);
  neg = pos == 1 ? 0 : 1;
}

int TablaDeAdaptaciones::MejorEtiqueta(int ejemplo, int variable) const {
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

}  // namespace inslv
