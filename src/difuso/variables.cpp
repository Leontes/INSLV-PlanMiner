// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/variables.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "inslv/comun/progreso.hpp"
#include "inslv/difuso/dominio.hpp"
#include "inslv/difuso/etiqueta.hpp"

namespace inslv::difuso {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Cuántas etiquetas admite (los 1 de la cadena).
int etiquetas_activas(const std::string& sub) {
  return static_cast<int>(std::count(sub.begin(), sub.end(), '1'));
}

}  // namespace

Variables::Variables(const Datos& data) {
  const std::size_t numero = data.atributos.size() + 1;
  variables_.reserve(numero);
  for (std::size_t v = 0; v < numero; ++v) {
    const Variable& variable = v + 1 == numero ? data.clase : data.atributos[v];
    std::vector<Etiqueta> etiquetas;
    etiquetas.reserve(variable.etiquetas.size());
    for (const ConjuntoDifuso& etiqueta : variable.etiquetas) {
      etiquetas.emplace_back(etiqueta.a, etiqueta.b, etiqueta.c, etiqueta.d, etiqueta.etiqueta);
    }
    variables_.emplace_back(variable.nombre, std::move(etiquetas));
  }
}

int Variables::numero_de_variables() const { return static_cast<int>(variables_.size()); }

int Variables::numero_de_atributos() const { return std::max(numero_de_variables() - 1, 0); }

int Variables::consecuente() const { return numero_de_variables() - 1; }

int Variables::numero_de_etiquetas(int variable) const {
  return variables_[indice(variable)].dominio().numero_de_etiquetas();
}

int Variables::etiquetas_de_atributos() const {
  int total = 0;
  for (int v = 0; v < numero_de_atributos(); ++v) {
    total += numero_de_etiquetas(v);
  }
  return total;
}

const std::string& Variables::nombre(int variable) const {
  return variables_[indice(variable)].nombre();
}

const std::string& Variables::nombre_de_etiqueta(int variable, int etiqueta) const {
  return variables_[indice(variable)].dominio().nombre_de_etiqueta(etiqueta);
}

void Variables::pintar() const {
  for (const VariableDifusa& variable : variables_) {
    comun::progreso(Verbosidad::depuracion) << '\n';
    variable.pintar();
    comun::progreso(Verbosidad::depuracion) << "----------\n";
  }
}

void Variables::pintar_nombre(int variable) const {
  comun::progreso(Verbosidad::depuracion) << nombre(variable);
}

double Variables::adaptacion(double x, int variable, int etiqueta) const {
  if (variable < 0 || indice(variable) >= variables_.size()) {
    throw std::logic_error("inslv: no existe la variable " + std::to_string(variable));
  }
  return variables_[indice(variable)].dominio().adaptacion(x, etiqueta);
}

double Variables::adaptacion(const datos::Ejemplo& x, const std::string& etiquetas,
                             const std::vector<double>& niveles) const {
  double max = 1;
  std::size_t trozo = 0;
  for (int i = 0; i < numero_de_atributos() && max > 0; ++i) {
    const Dominio& dominio = variables_[indice(i)].dominio();
    const int tam = dominio.numero_de_etiquetas();
    if (niveles[indice(i)] >= niveles.back()) {
      const std::string sub = etiquetas.substr(trozo, indice(tam));
      const int unos = etiquetas_activas(sub);
      if (unos == 0) {
        max = 0;
      } else if (unos < tam) {
        max = std::min(max, dominio.adaptacion(x.valor(i), sub));
      }
    }
    trozo += indice(tam);
  }
  return max;
}

bool Variables::es_valida(const std::string& etiquetas, const std::vector<double>& niveles) const {
  std::size_t trozo = 0;
  for (int i = 0; i < numero_de_atributos(); ++i) {
    const auto tam = indice(numero_de_etiquetas(i));
    if (niveles[indice(i)] >= niveles.back() &&
        etiquetas_activas(etiquetas.substr(trozo, tam)) == 0) {
      return false;
    }
    trozo += tam;
  }
  return true;
}

}  // namespace inslv::difuso
