// SPDX-License-Identifier: GPL-3.0-only
// Conversión entre los tipos de la librería (inslv) y los de old/ (inslv_legacy), que son los
// mismos con los nombres en inglés y otro espacio de nombres.
#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "inslv/inslv.hpp"
#include "inslv_legacy.hpp"

namespace inslv::pruebas {

inline inslv_legacy::Variable a_legacy(const Variable& variable) {
  inslv_legacy::Variable resultado;
  resultado.name = variable.nombre;
  for (const ConjuntoDifuso& etiqueta : variable.etiquetas) {
    resultado.labels.push_back({etiqueta.etiqueta, etiqueta.a, etiqueta.b, etiqueta.c, etiqueta.d});
  }
  return resultado;
}

inline inslv_legacy::Dataset a_legacy(const Datos& datos) {
  inslv_legacy::Dataset resultado;
  for (const Variable& atributo : datos.atributos) {
    resultado.attributes.push_back(a_legacy(atributo));
  }
  resultado.target = a_legacy(datos.clase);
  resultado.values = datos.valores;
  resultado.classes = datos.clases;
  return resultado;
}

inline inslv_legacy::Options a_legacy(const Opciones& opciones) {
  inslv_legacy::Options resultado;
  resultado.seed = opciones.semilla;
  resultado.report = opciones.informe;
  resultado.rules = opciones.reglas == TipoDeReglas::descriptivas
                        ? inslv_legacy::RuleType::descriptive
                        : inslv_legacy::RuleType::discriminant;
  resultado.episode_size = opciones.tamano_episodio;
  return resultado;
}

inline std::vector<Regla> de_legacy(const std::vector<inslv_legacy::Rule>& reglas) {
  std::vector<Regla> resultado;
  for (const inslv_legacy::Rule& regla : reglas) {
    Regla convertida;
    for (const inslv_legacy::Condition& condicion : regla.antecedent) {
      convertida.antecedente.push_back({condicion.attribute, condicion.labels});
    }
    convertida.consecuente = regla.consequent;
    convertida.cubiertos = regla.covered;
    convertida.peso = regla.weight;
    resultado.push_back(convertida);
  }
  return resultado;
}

// Lo que da old/: sus reglas y las clases que da su inferencia (ruleset::InferenceC) a otros
// ejemplos con ellas.
struct ResultadoDeOld {
  std::vector<Regla> reglas;
  std::vector<std::optional<std::size_t>> clases;
};

inline ResultadoDeOld aprender_con_old(const Datos& datos, const Opciones& opciones,
                                       const Datos& a_clasificar) {
  const inslv_legacy::Dataset otros = a_legacy(a_clasificar);
  std::vector<long> clases;
  inslv_legacy::Options opciones_de_old = a_legacy(opciones);
  opciones_de_old.classify = &otros;
  opciones_de_old.predictions = &clases;
  ResultadoDeOld resultado;
  resultado.reglas = de_legacy(inslv_legacy::learn(a_legacy(datos), opciones_de_old));
  for (const long clase : clases) {
    resultado.clases.push_back(
        clase < 0 ? std::nullopt : std::optional<std::size_t>(static_cast<std::size_t>(clase)));
  }
  return resultado;
}

}  // namespace inslv::pruebas
