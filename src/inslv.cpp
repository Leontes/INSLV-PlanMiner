// SPDX-License-Identifier: GPL-3.0-only
// La interfaz (inslv.hpp) sobre la implementación (difuso::Variables, datos::Ejemplos y Aprender).

#include "inslv/inslv.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/aprendizaje/aprender.hpp"
#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

// El valor que marca un dato desconocido en la implementación (datos::Ejemplos::MISSING y
// fuzzy_t::MISSING, privados): se adapta por completo a todas las etiquetas.
constexpr double desconocido = -999999999;

// a <= b <= c <= d. Escrito así, también es falso si alguno es NaN.
bool es_trapecio(const ConjuntoDifuso& etiqueta) {
  return etiqueta.a <= etiqueta.b && etiqueta.b <= etiqueta.c && etiqueta.c <= etiqueta.d;
}

// Si el valor se adapta algo a la etiqueta: como fuzzy_t::Adaptacion, que da 0 fuera de (a, d)
// salvo en [b, c].
bool se_adapta(double valor, const ConjuntoDifuso& etiqueta) {
  return (etiqueta.a < valor && valor < etiqueta.d) || (etiqueta.b <= valor && valor <= etiqueta.c);
}

void comprobar_etiquetas(const Variable& variable) {
  if (variable.etiquetas.empty()) {
    throw std::invalid_argument("inslv: la variable '" + variable.nombre + "' no tiene etiquetas");
  }
  for (const ConjuntoDifuso& etiqueta : variable.etiquetas) {
    if (!es_trapecio(etiqueta)) {
      throw std::invalid_argument("inslv: la etiqueta '" + etiqueta.etiqueta + "' de '" +
                                  variable.nombre + "' no cumple a <= b <= c <= d");
    }
  }
}

// Cada valor conocido tiene que adaptarse a alguna etiqueta de su atributo: si no, la
// implementación no puede calcular su adaptación (en el original terminaba el programa).
void comprobar_valores(const Datos& datos) {
  for (std::size_t i = 0; i < datos.atributos.size(); ++i) {
    const Variable& atributo = datos.atributos[i];
    for (std::size_t e = 0; e < datos.valores[i].size(); ++e) {
      const double valor = datos.valores[i][e];
      if (std::isnan(valor) || valor == desconocido) {
        continue;
      }
      if (std::none_of(
              atributo.etiquetas.begin(), atributo.etiquetas.end(),
              [valor](const ConjuntoDifuso& etiqueta) { return se_adapta(valor, etiqueta); })) {
        throw std::invalid_argument("inslv: el valor " + std::to_string(valor) + " del ejemplo " +
                                    std::to_string(e) + " en '" + atributo.nombre +
                                    "' no se adapta a ninguna de sus etiquetas");
      }
    }
  }
}

void comprobar(const Datos& datos) {
  if (datos.atributos.empty()) {
    throw std::invalid_argument("inslv: no hay atributos");
  }
  for (const Variable& atributo : datos.atributos) {
    comprobar_etiquetas(atributo);
  }
  comprobar_etiquetas(datos.clase);

  if (datos.valores.size() != datos.atributos.size()) {
    throw std::invalid_argument("inslv: hay " + std::to_string(datos.atributos.size()) +
                                " atributos y " + std::to_string(datos.valores.size()) +
                                " columnas de valores");
  }
  const std::size_t ejemplos = datos.clases.size();
  if (ejemplos == 0) {
    throw std::invalid_argument("inslv: no hay ejemplos");
  }
  for (std::size_t i = 0; i < datos.valores.size(); ++i) {
    if (datos.valores[i].size() != ejemplos) {
      throw std::invalid_argument("inslv: el atributo '" + datos.atributos[i].nombre + "' tiene " +
                                  std::to_string(datos.valores[i].size()) + " valores y hay " +
                                  std::to_string(ejemplos) + " ejemplos");
    }
  }
  for (const std::size_t clase : datos.clases) {
    if (clase >= datos.clase.etiquetas.size()) {
      throw std::invalid_argument("inslv: clase " + std::to_string(clase) + " fuera de rango");
    }
  }
  comprobar_valores(datos);
}

}  // namespace

std::vector<Regla> aprender(const Datos& datos, const Opciones& opciones) {
  comprobar(datos);
  if (opciones.tamano_episodio == 0 ||
      opciones.tamano_episodio > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    throw std::invalid_argument("inslv: el tamaño del episodio tiene que estar entre 1 y " +
                                std::to_string(std::numeric_limits<int>::max()));
  }

  // datos::Ejemplos recibe las columnas de todas las variables, con la clase al final. Los
  // desconocidos se marcan con su valor y cada clase, con el centro de su etiqueta.
  std::vector<std::vector<double>> columnas = datos.valores;
  for (std::vector<double>& columna : columnas) {
    for (double& valor : columna) {
      if (std::isnan(valor)) {
        valor = desconocido;
      }
    }
  }
  std::vector<double> clases;
  clases.reserve(datos.clases.size());
  for (const std::size_t clase : datos.clases) {
    const ConjuntoDifuso& etiqueta = datos.clase.etiquetas[clase];
    clases.push_back((etiqueta.b + etiqueta.c) / 2.0);
  }
  columnas.push_back(std::move(clases));

  const comun::DestinoDelProgreso destino(
      opciones.progreso != nullptr ? opciones.progreso : &std::cout, opciones.verbosidad);
  const std::size_t episodios =
      (datos.clases.size() + opciones.tamano_episodio - 1) / opciones.tamano_episodio;
  comun::progreso(Verbosidad::resumen) << std::format(
      "INSLV: {} ejemplos, {} atributos y {} clases; {} {} de hasta {} ejemplos; reglas {}\n",
      datos.clases.size(), datos.atributos.size(), datos.clase.etiquetas.size(), episodios,
      episodios == 1 ? "episodio" : "episodios", opciones.tamano_episodio,
      opciones.reglas == TipoDeReglas::descriptivas ? "descriptivas" : "discriminantes");

  const auto inicio = std::chrono::steady_clock::now();
  const difuso::Variables variables(datos);
  const datos::Ejemplos ejemplos(columnas);
  std::vector<Regla> reglas = aprendizaje::aprender(variables, ejemplos, opciones);
  const std::chrono::duration<double> segundos = std::chrono::steady_clock::now() - inicio;

  comun::progreso(Verbosidad::resumen)
      << std::format("INSLV: {} reglas en {:.2f} s\n", reglas.size(), segundos.count());
  return reglas;
}

}  // namespace inslv
