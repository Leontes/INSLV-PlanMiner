// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/tabla.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

TablaDeAdaptaciones::TablaDeAdaptaciones(const difuso::Variables& variables,
                                         const datos::Ejemplos& ejemplos,
                                         std::size_t memoria_maxima)
    : n_ejemplos_(indice(ejemplos.numero_de_ejemplos())),
      n_atributos_(variables.numero_de_atributos()),
      consecuente_(variables.consecuente()),
      conjuntos_(indice(variables.numero_de_variables())),
      conjuntos_maximos_(memoria_maxima / std::max<std::size_t>(n_ejemplos_ * sizeof(double), 1)) {
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

const std::vector<double>& TablaDeAdaptaciones::adaptaciones_a_clase(int clase) const {
  return adaptaciones_[indice(consecuente_)][indice(clase)];
}

const std::vector<double>& TablaDeAdaptaciones::adaptaciones_a_variable(
    int variable, std::string_view etiquetas) const {
  ConjuntosDeEtiquetas& conjuntos = conjuntos_[indice(variable)];
  const auto guardado = conjuntos.find(etiquetas);
  if (guardado != conjuntos.end()) {
    return guardado->second;
  }

  if (conjuntos_guardados_ >= conjuntos_maximos_) {
    for (ConjuntosDeEtiquetas& de_una_variable : conjuntos_) {
      de_una_variable.clear();
    }
    conjuntos_guardados_ = 0;
  }

  const std::vector<std::vector<double>>& tabla = adaptaciones_[indice(variable)];
  std::vector<double> resultado(n_ejemplos_, 0);
  for (std::size_t etiqueta = 0; etiqueta < tabla.size(); ++etiqueta) {
    if (etiquetas[etiqueta] != '1') {
      continue;
    }
    const std::vector<double>& columna = tabla[etiqueta];
    for (std::size_t e = 0; e < n_ejemplos_; ++e) {
      if (resultado[e] == 0) {
        resultado[e] = columna[e];
      } else if (resultado[e] < 1 && columna[e] > 0) {
        resultado[e] = 1;
      }
    }
  }
  ++conjuntos_guardados_;
  return conjuntos.emplace(etiquetas, std::move(resultado)).first->second;
}

void TablaDeAdaptaciones::adaptaciones_al_antecedente(const std::string& etiquetas,
                                                      const std::vector<double>& niveles,
                                                      std::vector<double>& salida) const {
  salida.assign(n_ejemplos_, 1);
  const std::string_view todas = etiquetas;
  std::size_t trozo = 0;
  for (int v = 0; v < n_atributos_; ++v) {
    const std::size_t tam = adaptaciones_[indice(v)].size();
    if (niveles[indice(v)] >= niveles.back()) {
      const std::vector<double>& columna = adaptaciones_a_variable(v, todas.substr(trozo, tam));
      for (std::size_t e = 0; e < n_ejemplos_; ++e) {
        salida[e] = columna[e] < salida[e] ? columna[e] : salida[e];
      }
    }
    trozo += tam;
  }
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
