// SPDX-License-Identifier: GPL-3.0-only
// Conversión entre los tipos de la librería (inslv) y los de old/ (inslv_legacy), que son los
// mismos con otro espacio de nombres.
#pragma once

#include <vector>

#include "inslv/inslv.hpp"
#include "inslv_legacy.hpp"

namespace inslv::pruebas {

inline inslv_legacy::Variable a_legacy(const Variable& variable) {
  inslv_legacy::Variable resultado;
  resultado.name = variable.name;
  for (const FuzzySet& etiqueta : variable.labels) {
    resultado.labels.push_back({etiqueta.label, etiqueta.a, etiqueta.b, etiqueta.c, etiqueta.d});
  }
  return resultado;
}

inline inslv_legacy::Dataset a_legacy(const Dataset& datos) {
  inslv_legacy::Dataset resultado;
  for (const Variable& atributo : datos.attributes) {
    resultado.attributes.push_back(a_legacy(atributo));
  }
  resultado.target = a_legacy(datos.target);
  resultado.values = datos.values;
  resultado.classes = datos.classes;
  return resultado;
}

inline inslv_legacy::Options a_legacy(const Options& opciones) {
  inslv_legacy::Options resultado;
  resultado.seed = opciones.seed;
  resultado.verbose = opciones.verbose;
  resultado.report = opciones.report;
  resultado.rules = opciones.rules == RuleType::descriptive ? inslv_legacy::RuleType::descriptive
                                                            : inslv_legacy::RuleType::discriminant;
  resultado.episode_size = opciones.episode_size;
  return resultado;
}

inline std::vector<Rule> de_legacy(const std::vector<inslv_legacy::Rule>& reglas) {
  std::vector<Rule> resultado;
  for (const inslv_legacy::Rule& regla : reglas) {
    Rule convertida;
    for (const inslv_legacy::Condition& condicion : regla.antecedent) {
      convertida.antecedent.push_back({condicion.attribute, condicion.labels});
    }
    convertida.consequent = regla.consequent;
    convertida.covered = regla.covered;
    convertida.weight = regla.weight;
    resultado.push_back(convertida);
  }
  return resultado;
}

}  // namespace inslv::pruebas
