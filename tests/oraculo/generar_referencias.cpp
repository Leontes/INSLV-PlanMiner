// SPDX-License-Identifier: GPL-3.0-only
// Genera ficheros de referencia (tests/datos/referencias) con old/: aprende el experimento que
// dice el nombre de cada fichero y escribe las reglas y las clases que da su inferencia a la
// partición de test. Solo en local, como el oráculo.
//
//   inslv_generar_referencias <fichero.reglas>...
//
// old/ usa el rand() global y desvía std::cout: para generar varios a la vez, un proceso por
// fichero (xargs -P).

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <vector>

#include "conversion.hpp"
#include "referencias.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "uso: " << argv[0] << " <fichero.reglas>...\n";
    return 2;
  }
  try {
    const std::vector<char*> ficheros(argv + 1, argv + argc);
    for (const char* const ruta : ficheros) {
      const std::filesystem::path fichero = ruta;
      const inslv::pruebas::Experimento experimento = inslv::pruebas::experimento_de(fichero);
      const auto inicio = std::chrono::steady_clock::now();
      const inslv::pruebas::ResultadoDeOld resultado = inslv::pruebas::aprender_con_old(
          experimento.entrenamiento(), experimento.opciones(), experimento.prueba());
      const std::chrono::duration<double> segundos = std::chrono::steady_clock::now() - inicio;
      inslv::pruebas::escribir_reglas(fichero, resultado.reglas, resultado.clases);
      std::cout << fichero.filename().string() << ": " << resultado.reglas.size() << " reglas en "
                << segundos.count() << " s\n";
    }
  } catch (const std::exception& error) {
    std::cerr << "inslv_generar_referencias: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
