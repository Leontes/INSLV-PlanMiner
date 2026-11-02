// SPDX-License-Identifier: GPL-3.0-only
// Las reglas de referencia (tests/datos/referencias): las que da old/ con un conjunto de NSLV y
// unas opciones, y las clases que da su inferencia a los ejemplos de la partición de test. El
// nombre del fichero dice cuáles:
//
//   <conjunto>-test<k>-<descriptivas|discriminantes>[-episodio<n>].reglas
//
// Se aprende con todas las particiones de tests/datos/nslv/<conjunto> salvo la k, con la semilla
// por defecto y episodios de n ejemplos (5000 si no se dice). Los genera
// inslv_generar_referencias (tests/oraculo), que necesita old/.
#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "inslv/inslv.hpp"

namespace inslv::pruebas {

struct Experimento {
  std::string conjunto;
  /// La partición que no se usa para aprender.
  int test = 0;
  TipoDeReglas tipo = TipoDeReglas::descriptivas;
  std::size_t episodio = 5000;

  /// Los ejemplos con los que se aprende: todas las particiones salvo la de test.
  [[nodiscard]] Datos entrenamiento() const;
  /// Los ejemplos de la partición de test.
  [[nodiscard]] Datos prueba() const;
  [[nodiscard]] Opciones opciones() const;
};

/// El experimento de un fichero de referencia, por su nombre. Lanza std::runtime_error si el
/// nombre no sigue el formato.
Experimento experimento_de(const std::filesystem::path& fichero);

/// Las reglas como texto: "reglas <n>" y, por cada regla, "regla <clase> <cubiertos> <peso>
/// <condiciones>" y una línea "condicion <atributo> <etiquetas>..." por condición. Los números
/// van con la representación más corta que se lee igual: dos textos iguales son reglas iguales
/// bit a bit.
std::string describir(const std::vector<Regla>& reglas);

/// Las clases de una clasificación como texto: "predicciones <n>" y una línea con la clase de
/// cada ejemplo, o -1 si no tiene.
std::string describir(const std::vector<std::optional<std::size_t>>& clases);

/// Escribe un fichero de referencia: una cabecera con el experimento, las reglas y las clases de
/// la partición de test.
void escribir_reglas(const std::filesystem::path& fichero, const std::vector<Regla>& reglas,
                     const std::vector<std::optional<std::size_t>>& clases);

/// Lee las reglas de un fichero de referencia. Lanza std::runtime_error si no sigue el formato.
std::vector<Regla> leer_reglas(const std::filesystem::path& fichero);

/// Lee las clases de la partición de test de un fichero de referencia.
std::vector<std::optional<std::size_t>> leer_predicciones(const std::filesystem::path& fichero);

/// Los ficheros de referencia de tests/datos/referencias/<grupo>, en orden alfabético.
std::vector<std::filesystem::path> ficheros_de_referencia(const std::string& grupo);

}  // namespace inslv::pruebas
