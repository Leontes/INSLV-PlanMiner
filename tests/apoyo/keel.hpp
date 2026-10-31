// SPDX-License-Identifier: GPL-3.0-only
// Lectura mínima del formato de KEEL, para comparar con el lector de NSLV en los tests.
//
// De cada atributo solo se lee el nombre y, si es nominal, sus valores; de cada ejemplo, los
// valores como texto ("?" si es desconocido).
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace inslv::pruebas {

struct AtributoKeel {
  std::string nombre;
  /// Los valores de un atributo nominal, en el orden en que se declaran; vacío si es numérico.
  std::vector<std::string> valores;
};

struct DatosKeel {
  std::vector<AtributoKeel> atributos;
  std::vector<std::vector<std::string>> ejemplos;
};

/// Lee los atributos del primer fichero y los ejemplos de todos, uno tras otro. Lanza
/// std::runtime_error si un fichero no se puede abrir o tiene un ejemplo con otro número de
/// valores.
DatosKeel leer_keel(const std::vector<std::filesystem::path>& ficheros);

}  // namespace inslv::pruebas
