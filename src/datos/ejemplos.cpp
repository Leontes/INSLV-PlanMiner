// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/datos/ejemplos.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace inslv::datos {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

Ejemplos::Ejemplos(int ejemplos, int variables) {
  // En el original solo avisaba y dejaba los punteros sin inicializar.
  if (ejemplos < 0 || variables <= 0) {
    throw std::logic_error("inslv: conjunto con " + std::to_string(ejemplos) + " ejemplos y " +
                           std::to_string(variables) + " variables");
  }
  n_variables_ = variables;
  const std::size_t n = indice(ejemplos);
  valores_.assign(n, std::vector<double>(indice(variables)));
  bien_clasificado_.assign(n, false);
  grado_positivo_.assign(n, 0);
  grado_negativo_.assign(n, 0);
  peso_positivo_.assign(n, 0);
  peso_negativo_.assign(n, 0);
  regla_usada_.assign(n, -1);
  regla_aislada_.assign(n, 0);
}

Ejemplos::Ejemplos(const std::vector<std::vector<double>>& columnas)
    : Ejemplos(static_cast<int>(columnas[0].size()), static_cast<int>(columnas.size())) {
  for (std::size_t v = 0; v < columnas.size(); ++v) {
    for (std::size_t e = 0; e < columnas[v].size(); ++e) {
      valores_[e][v] = columnas[v][e];
    }
  }
}

double Ejemplos::valor(int ejemplo, int variable) const {
  return valores_[indice(ejemplo)][indice(variable)];
}

Ejemplo Ejemplos::ejemplo(int i) const { return Ejemplo(valores_[indice(i)]); }

int Ejemplos::numero_de_ejemplos() const { return static_cast<int>(valores_.size()); }

int Ejemplos::sin_clasificar_bien() const {
  return static_cast<int>(std::count(bien_clasificado_.begin(), bien_clasificado_.end(), false));
}

std::vector<int> Ejemplos::sin_clasificar_bien_por_clase(int variable_clase, int clases) const {
  std::vector<int> por_clase(indice(clases), 0);
  for (std::size_t i = 0; i < valores_.size(); ++i) {
    if (!bien_clasificado_[i]) {
      ++por_clase[static_cast<std::size_t>(valores_[i][indice(variable_clase)])];
    }
  }
  return por_clase;
}

void Ejemplos::anotar(int i, bool bien_clasificado, double grado_p, double peso_p, double grado_n,
                      double peso_n, int regla, int aislada) {
  const std::size_t e = indice(i);
  bien_clasificado_[e] = bien_clasificado;
  grado_positivo_[e] = grado_p;
  peso_positivo_[e] = peso_p;
  grado_negativo_[e] = grado_n;
  peso_negativo_[e] = peso_n;
  regla_usada_[e] = regla;
  regla_aislada_[e] = aislada;
}

Ejemplos Ejemplos::primeros(int n) const { return subconjunto(0, n - 1); }

Ejemplos Ejemplos::subconjunto(int ini, int fin) const {
  const int desde = std::max(ini, 0);
  const int hasta = std::min(fin, numero_de_ejemplos() - 1);
  Ejemplos resultado(std::max(hasta - desde + 1, 0), n_variables_);
  for (int e = desde; e <= hasta; ++e) {
    resultado.valores_[indice(e - desde)] = valores_[indice(e)];
  }
  return resultado;
}

}  // namespace inslv::datos
