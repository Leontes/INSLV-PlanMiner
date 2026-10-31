// SPDX-License-Identifier: GPL-3.0-only
#include "keel.hpp"

#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace inslv::pruebas {

namespace {

std::string sin_espacios(const std::string& texto) {
  const std::size_t inicio = texto.find_first_not_of(" \t\r");
  if (inicio == std::string::npos) {
    return "";
  }
  return texto.substr(inicio, texto.find_last_not_of(" \t\r") - inicio + 1);
}

std::vector<std::string> separar(const std::string& texto) {
  std::vector<std::string> partes;
  std::istringstream entrada(texto);
  std::string parte;
  while (std::getline(entrada, parte, ',')) {
    partes.push_back(sin_espacios(parte));
  }
  return partes;
}

// @attribute <nombre> real | integer | {v1,v2,...}
AtributoKeel leer_atributo(const std::string& linea) {
  std::istringstream entrada(linea);
  std::string etiqueta;
  AtributoKeel atributo;
  entrada >> etiqueta >> atributo.nombre;
  std::string tipo;
  std::getline(entrada, tipo);
  tipo = sin_espacios(tipo);
  if (tipo.starts_with('{')) {
    atributo.valores = separar(tipo.substr(1, tipo.find('}') - 1));
  }
  return atributo;
}

}  // namespace

DatosKeel leer_keel(const std::vector<std::filesystem::path>& ficheros) {
  DatosKeel datos;
  for (const std::filesystem::path& fichero : ficheros) {
    std::ifstream entrada(fichero);
    if (!entrada) {
      throw std::runtime_error("no se puede abrir " + fichero.string());
    }
    const bool primero = datos.atributos.empty();
    bool en_datos = false;
    std::string linea;
    while (std::getline(entrada, linea)) {
      linea = sin_espacios(linea);
      if (linea.empty()) {
        continue;
      }
      if (en_datos) {
        std::vector<std::string> ejemplo = separar(linea);
        if (ejemplo.size() != datos.atributos.size()) {
          throw std::runtime_error(fichero.string() + ": un ejemplo tiene " +
                                   std::to_string(ejemplo.size()) + " valores: " + linea);
        }
        datos.ejemplos.push_back(std::move(ejemplo));
      } else if (linea.starts_with("@attribute") && primero) {
        datos.atributos.push_back(leer_atributo(linea));
      } else if (linea.starts_with("@data")) {
        en_datos = true;
      }
    }
  }
  return datos;
}

}  // namespace inslv::pruebas
