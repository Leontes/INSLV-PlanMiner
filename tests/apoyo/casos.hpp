// SPDX-License-Identifier: GPL-3.0-only
// Los casos de equivalencia de PlanMiner (tests/datos/planminer).
//
// Cada caso es una llamada a inslv() del PlanMiner original: la entrada (variables, etiquetas y
// ejemplos) y las reglas que devolvió, ya interpretadas por PlanMiner. Se generaron con el
// PlanMiner original y no se pueden regenerar fuera de él.
#pragma once

#include <filesystem>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "inslv/inslv.hpp"

namespace inslv::pruebas {

/// Una regla como la daba el PlanMiner original: los ejemplos cubiertos, con signo negativo si
/// la clase es PreState, y los pares (atributo, valor).
struct ReglaPlanMiner {
  int cubiertos = 0;
  std::vector<std::pair<std::string, std::string>> condiciones;

  bool operator==(const ReglaPlanMiner& otra) const = default;
};

std::ostream& operator<<(std::ostream& salida, const ReglaPlanMiner& regla);

struct CasoPlanMiner {
  Dataset datos;
  std::vector<ReglaPlanMiner> reglas;
};

/// Lee un caso. Lanza std::runtime_error si el formato no es el esperado.
CasoPlanMiner leer_caso(const std::filesystem::path& fichero);

/// Los ficheros de los casos, en orden alfabético.
std::vector<std::filesystem::path> ficheros_de_casos();

}  // namespace inslv::pruebas
