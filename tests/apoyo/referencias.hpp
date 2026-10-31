// SPDX-License-Identifier: GPL-3.0-only
// Las reglas de referencia (tests/datos/referencias): las que da old/ con un conjunto de NSLV y
// unas opciones. El nombre del fichero dice cuáles:
//
//   <conjunto>-test<k>-<descriptivas|discriminantes>[-episodio<n>].reglas
//
// Se aprende con todas las particiones de tests/datos/nslv/<conjunto> salvo la k, con la semilla
// por defecto y episodios de n ejemplos (5000 si no se dice). Los genera
// inslv_generar_referencias (tests/oraculo), que necesita old/.
#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "inslv/inslv.hpp"

namespace inslv::pruebas {

struct Experimento {
  std::string conjunto;
  /// La partición que no se usa para aprender.
  int test = 0;
  RuleType tipo = RuleType::descriptive;
  std::size_t episodio = 5000;

  /// Los ejemplos con los que se aprende: todas las particiones salvo la de test.
  [[nodiscard]] Dataset entrenamiento() const;
  [[nodiscard]] Options opciones() const;
};

/// El experimento de un fichero de referencia, por su nombre. Lanza std::runtime_error si el
/// nombre no sigue el formato.
Experimento experimento_de(const std::filesystem::path& fichero);

/// Las reglas como texto: "reglas <n>" y, por cada regla, "regla <clase> <cubiertos> <peso>
/// <condiciones>" y una línea "condicion <atributo> <etiquetas>..." por condición. Los números
/// van con la representación más corta que se lee igual: dos textos iguales son reglas iguales
/// bit a bit.
std::string describir(const std::vector<Rule>& reglas);

/// Escribe un fichero de referencia: una cabecera con el experimento y las reglas.
void escribir_reglas(const std::filesystem::path& fichero, const std::vector<Rule>& reglas);

/// Lee las reglas de un fichero de referencia. Lanza std::runtime_error si no sigue el formato.
std::vector<Rule> leer_reglas(const std::filesystem::path& fichero);

/// Los ficheros de referencia de tests/datos/referencias/<grupo>, en orden alfabético.
std::vector<std::filesystem::path> ficheros_de_referencia(const std::string& grupo);

}  // namespace inslv::pruebas
