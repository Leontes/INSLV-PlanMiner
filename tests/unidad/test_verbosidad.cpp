// SPDX-License-Identifier: GPL-3.0-only
// La verbosidad: cuánto cuenta aprender de lo que hace y adónde lo escribe.

#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "inslv/formatos/nslv.hpp"
#include "inslv/inslv.hpp"
#include "referencias.hpp"

namespace {

// irs sin la partición 0 (135 ejemplos), en dos episodios de 70.
inslv::Datos irs() {
  inslv::pruebas::Experimento experimento;
  experimento.conjunto = "irs";
  return experimento.entrenamiento();
}

struct Salida {
  std::string texto;
  std::vector<inslv::Regla> reglas;
};

Salida aprender_con(inslv::Verbosidad verbosidad) {
  std::ostringstream flujo;
  inslv::Opciones opciones;
  opciones.verbosidad = verbosidad;
  opciones.progreso = &flujo;
  opciones.tamano_episodio = 70;
  Salida salida;
  salida.reglas = inslv::aprender(irs(), opciones);
  salida.texto = flujo.str();
  return salida;
}

std::vector<std::string> lineas(const std::string& texto) {
  std::vector<std::string> resultado;
  std::istringstream entrada(texto);
  std::string linea;
  while (std::getline(entrada, linea)) {
    resultado.push_back(linea);
  }
  return resultado;
}

TEST(Verbosidad, NingunaNoEscribeNada) {
  testing::internal::CaptureStdout();
  EXPECT_EQ(aprender_con(inslv::Verbosidad::ninguna).texto, "");
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
}

TEST(Verbosidad, ResumenEsUnaLineaAlEmpezarYOtraAlAcabar) {
  const Salida salida = aprender_con(inslv::Verbosidad::resumen);
  const std::vector<std::string> escritas = lineas(salida.texto);
  ASSERT_EQ(escritas.size(), 2U) << salida.texto;
  EXPECT_EQ(escritas[0],
            "INSLV: 135 ejemplos, 4 atributos y 3 clases; 2 episodios de hasta 70 ejemplos; "
            "reglas descriptivas");
  EXPECT_TRUE(
      escritas[1].starts_with("INSLV: " + std::to_string(salida.reglas.size()) + " reglas en "))
      << escritas[1];
}

TEST(Verbosidad, EpisodiosAnadeUnaLineaPorEpisodio) {
  const std::vector<std::string> escritas =
      lineas(aprender_con(inslv::Verbosidad::episodios).texto);
  ASSERT_EQ(escritas.size(), 4U);
  EXPECT_TRUE(escritas[1].starts_with("INSLV: episodio 1 de 2 (ejemplos 0 a 69): reglas 0 -> "))
      << escritas[1];
  EXPECT_TRUE(escritas[2].starts_with("INSLV: episodio 2 de 2 (ejemplos 70 a 134): reglas "))
      << escritas[2];
}

// Las líneas sin los números, que cambian de una ejecución a otra (los tiempos).
std::vector<std::string> sin_numeros(const std::vector<std::string>& escritas) {
  std::vector<std::string> resultado;
  resultado.reserve(escritas.size());
  for (const std::string& linea : escritas) {
    resultado.push_back(std::regex_replace(linea, std::regex("[0-9.e+-]+"), "#"));
  }
  return resultado;
}

TEST(Verbosidad, CadaNivelIncluyeLosAnteriores) {
  std::vector<std::string> anterior;
  for (const inslv::Verbosidad verbosidad :
       {inslv::Verbosidad::resumen, inslv::Verbosidad::episodios, inslv::Verbosidad::detalle,
        inslv::Verbosidad::depuracion}) {
    const std::vector<std::string> escritas = sin_numeros(lineas(aprender_con(verbosidad).texto));
    EXPECT_GT(escritas.size(), anterior.size()) << static_cast<int>(verbosidad);
    // Las líneas del nivel anterior están, en el mismo orden.
    std::size_t encontradas = 0;
    for (const std::string& linea : escritas) {
      if (encontradas < anterior.size() && linea == anterior[encontradas]) {
        ++encontradas;
      }
    }
    EXPECT_EQ(encontradas, anterior.size()) << static_cast<int>(verbosidad);
    anterior = escritas;
  }
}

TEST(Verbosidad, NoCambiaLasReglas) {
  // Los mensajes del código heredado calculan cosas (Aciertos marca los ejemplos): se evalúan
  // siempre, aunque no se escriban.
  const std::string sin_mensajes =
      inslv::pruebas::describir(aprender_con(inslv::Verbosidad::ninguna).reglas);
  for (const inslv::Verbosidad verbosidad :
       {inslv::Verbosidad::resumen, inslv::Verbosidad::episodios, inslv::Verbosidad::detalle,
        inslv::Verbosidad::depuracion}) {
    EXPECT_EQ(inslv::pruebas::describir(aprender_con(verbosidad).reglas), sin_mensajes)
        << static_cast<int>(verbosidad);
  }
}

TEST(Verbosidad, SinFlujoVaALaSalidaEstandar) {
  inslv::Opciones opciones;
  opciones.verbosidad = inslv::Verbosidad::resumen;
  testing::internal::CaptureStdout();
  inslv::aprender(irs(), opciones);
  EXPECT_EQ(lineas(testing::internal::GetCapturedStdout()).size(), 2U);
}

TEST(Verbosidad, ElDestinoNoSeQuedaPuesto) {
  aprender_con(inslv::Verbosidad::depuracion);
  // Otra llamada sin verbosidad no escribe en ninguna parte.
  testing::internal::CaptureStdout();
  EXPECT_EQ(aprender_con(inslv::Verbosidad::ninguna).texto, "");
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
}

}  // namespace
