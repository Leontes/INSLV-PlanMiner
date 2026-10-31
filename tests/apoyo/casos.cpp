// SPDX-License-Identifier: GPL-3.0-only
#include "casos.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace inslv::pruebas {

namespace {

// El resto de la línea tras el último campo leído, sin el espacio que lo separa.
std::string resto_de_linea(std::istringstream& linea) {
  std::string resto;
  std::getline(linea >> std::ws, resto);
  return resto;
}

double leer_valor(const std::string& texto) {
  return texto == "?" ? std::numeric_limits<double>::quiet_NaN() : std::stod(texto);
}

// Lee la siguiente línea que no es un comentario y comprueba su palabra clave.
std::istringstream siguiente_linea(std::ifstream& fichero, const std::string& clave) {
  std::string texto;
  do {
    if (!std::getline(fichero, texto)) {
      throw std::runtime_error("fin de fichero; se esperaba '" + clave + "'");
    }
  } while (texto.empty() || texto.front() == '#');

  std::istringstream linea(texto);
  std::string palabra;
  linea >> palabra;
  if (palabra != clave) {
    throw std::runtime_error("se esperaba '" + clave + "' y hay: " + texto);
  }
  return linea;
}

Variable leer_variable(std::ifstream& fichero, const std::string& clave) {
  std::istringstream linea = siguiente_linea(fichero, clave);
  std::size_t etiquetas = 0;
  linea >> etiquetas;
  Variable variable;
  variable.name = resto_de_linea(linea);
  for (std::size_t i = 0; i < etiquetas; ++i) {
    std::istringstream linea_etiqueta = siguiente_linea(fichero, "label");
    std::string a;
    std::string b;
    std::string c;
    std::string d;
    FuzzySet etiqueta;
    linea_etiqueta >> a >> b >> c >> d >> etiqueta.label;
    etiqueta.a = leer_valor(a);
    etiqueta.b = leer_valor(b);
    etiqueta.c = leer_valor(c);
    etiqueta.d = leer_valor(d);
    variable.labels.push_back(etiqueta);
  }
  return variable;
}

}  // namespace

std::ostream& operator<<(std::ostream& salida, const ReglaPlanMiner& regla) {
  salida << "regla " << regla.cubiertos << ":";
  for (const auto& [atributo, valor] : regla.condiciones) {
    salida << " [" << atributo << " = " << valor << "]";
  }
  return salida;
}

CasoPlanMiner leer_caso(const std::filesystem::path& fichero) {
  std::ifstream entrada(fichero);
  if (!entrada) {
    throw std::runtime_error("no se puede abrir " + fichero.string());
  }
  CasoPlanMiner caso;

  std::size_t atributos = 0;
  siguiente_linea(entrada, "attributes") >> atributos;
  for (std::size_t i = 0; i < atributos; ++i) {
    caso.datos.attributes.push_back(leer_variable(entrada, "attribute"));
  }
  caso.datos.target = leer_variable(entrada, "target");

  std::size_t ejemplos = 0;
  siguiente_linea(entrada, "examples") >> ejemplos;
  caso.datos.values.assign(atributos, std::vector<double>(ejemplos));
  caso.datos.classes.resize(ejemplos);
  for (std::size_t e = 0; e < ejemplos; ++e) {
    std::string texto;
    std::getline(entrada, texto);
    std::istringstream linea(texto);
    for (std::size_t i = 0; i < atributos; ++i) {
      std::string valor;
      linea >> valor;
      caso.datos.values[i][e] = leer_valor(valor);
    }
    linea >> caso.datos.classes[e];
  }

  std::size_t reglas = 0;
  siguiente_linea(entrada, "rules") >> reglas;
  for (std::size_t r = 0; r < reglas; ++r) {
    std::istringstream linea = siguiente_linea(entrada, "rule");
    ReglaPlanMiner regla;
    std::size_t condiciones = 0;
    linea >> regla.cubiertos >> condiciones;
    for (std::size_t c = 0; c < condiciones; ++c) {
      std::istringstream condicion = siguiente_linea(entrada, "condition");
      std::string valor;
      condicion >> valor;
      regla.condiciones.emplace_back(resto_de_linea(condicion), valor);
    }
    caso.reglas.push_back(regla);
  }
  return caso;
}

std::vector<std::filesystem::path> ficheros_de_casos() {
  std::vector<std::filesystem::path> ficheros;
  for (const auto& entrada : std::filesystem::directory_iterator(INSLV_CASOS_PLANMINER)) {
    if (entrada.path().extension() == ".case") {
      ficheros.push_back(entrada.path());
    }
  }
  std::sort(ficheros.begin(), ficheros.end());
  return ficheros;
}

}  // namespace inslv::pruebas
