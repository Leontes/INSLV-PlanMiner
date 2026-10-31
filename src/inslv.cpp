// SPDX-License-Identifier: GPL-3.0-only
// La API de inslv.hpp sobre la implementación original (VectorVar, example_set y Aprender).

#include "inslv/inslv.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/aprendizaje/RuleCreator.hpp"
#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

// El valor que marca un dato desconocido en la implementación (example_set::MISSING y
// fuzzy_t::MISSING, privados): se adapta por completo a todas las etiquetas.
constexpr double missing_value = -999999999;

// a <= b <= c <= d. Escrito así, también es falso si alguno es NaN.
bool es_trapecio(const FuzzySet& etiqueta) {
  return etiqueta.a <= etiqueta.b && etiqueta.b <= etiqueta.c && etiqueta.c <= etiqueta.d;
}

// Si el valor se adapta algo a la etiqueta: como fuzzy_t::Adaptacion, que da 0 fuera de (a, d)
// salvo en [b, c].
bool se_adapta(double valor, const FuzzySet& etiqueta) {
  return (etiqueta.a < valor && valor < etiqueta.d) || (etiqueta.b <= valor && valor <= etiqueta.c);
}

void check_labels(const Variable& variable) {
  if (variable.labels.empty()) {
    throw std::invalid_argument("inslv: la variable '" + variable.name + "' no tiene etiquetas");
  }
  for (const FuzzySet& etiqueta : variable.labels) {
    if (!es_trapecio(etiqueta)) {
      throw std::invalid_argument("inslv: la etiqueta '" + etiqueta.label + "' de '" +
                                  variable.name + "' no cumple a <= b <= c <= d");
    }
  }
}

// Cada valor conocido tiene que adaptarse a alguna etiqueta de su atributo: si no, la
// implementación no puede calcular su adaptación (en el original terminaba el programa).
void check_values(const Dataset& data) {
  for (std::size_t i = 0; i < data.attributes.size(); ++i) {
    const Variable& atributo = data.attributes[i];
    for (std::size_t e = 0; e < data.values[i].size(); ++e) {
      const double valor = data.values[i][e];
      if (std::isnan(valor) || valor == missing_value) {
        continue;
      }
      if (std::none_of(atributo.labels.begin(), atributo.labels.end(),
                       [valor](const FuzzySet& etiqueta) { return se_adapta(valor, etiqueta); })) {
        throw std::invalid_argument("inslv: el valor " + std::to_string(valor) + " del ejemplo " +
                                    std::to_string(e) + " en '" + atributo.name +
                                    "' no se adapta a ninguna de sus etiquetas");
      }
    }
  }
}

void check(const Dataset& data) {
  if (data.attributes.empty()) {
    throw std::invalid_argument("inslv: no hay atributos");
  }
  for (const Variable& attribute : data.attributes) {
    check_labels(attribute);
  }
  check_labels(data.target);

  if (data.values.size() != data.attributes.size()) {
    throw std::invalid_argument("inslv: hay " + std::to_string(data.attributes.size()) +
                                " atributos y " + std::to_string(data.values.size()) +
                                " columnas de valores");
  }
  const std::size_t examples = data.classes.size();
  if (examples == 0) {
    throw std::invalid_argument("inslv: no hay ejemplos");
  }
  for (std::size_t i = 0; i < data.values.size(); ++i) {
    if (data.values[i].size() != examples) {
      throw std::invalid_argument("inslv: el atributo '" + data.attributes[i].name + "' tiene " +
                                  std::to_string(data.values[i].size()) + " valores y hay " +
                                  std::to_string(examples) + " ejemplos");
    }
  }
  for (const std::size_t c : data.classes) {
    if (c >= data.target.labels.size()) {
      throw std::invalid_argument("inslv: clase " + std::to_string(c) + " fuera de rango");
    }
  }
  check_values(data);
}

// Adónde va el progreso de la implementación: al flujo de las opciones, a la salida estándar con
// verbose o a ninguna parte.
std::ostream* destino_del_progreso(const Options& opciones) {
  if (opciones.progress != nullptr) {
    return opciones.progress;
  }
  return opciones.verbose ? &std::cout : nullptr;
}

}  // namespace

std::vector<Rule> learn(const Dataset& data, const Options& options) {
  check(data);
  if (options.episode_size == 0 ||
      options.episode_size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    throw std::invalid_argument("inslv: el tamaño del episodio tiene que estar entre 1 y " +
                                std::to_string(std::numeric_limits<int>::max()));
  }

  // example_set recibe las columnas de todas las variables, con la clase al final. Los
  // desconocidos se marcan con missing_value y cada clase, con el centro de su etiqueta.
  std::vector<std::vector<double>> columns = data.values;
  for (std::vector<double>& column : columns) {
    for (double& value : column) {
      if (std::isnan(value)) {
        value = missing_value;
      }
    }
  }
  std::vector<double> classes;
  classes.reserve(data.classes.size());
  for (const std::size_t c : data.classes) {
    const FuzzySet& label = data.target.labels[c];
    classes.push_back((label.b + label.c) / 2.0);
  }
  columns.push_back(std::move(classes));

  const DestinoDelProgreso destino(destino_del_progreso(options));
  VectorVar variables(data);
  example_set examples(&columns);
  return Aprender(variables, examples, options);
}

}  // namespace inslv
