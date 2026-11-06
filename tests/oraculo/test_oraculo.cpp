// SPDX-License-Identifier: GPL-3.0-only
// Comparación con old/, compilado en el mismo programa (inslv_legacy).
//
// La librería tiene que dar exactamente las mismas reglas que el original, descriptivas y
// discriminantes (antecedente, clase, ejemplos cubiertos y peso, bit a bit), y clasificar con ellas
// los ejemplos igual que su inferencia (ruleset::InferenceC). Es más estricto que la equivalencia
// con los casos de PlanMiner, que solo compara lo que interpretaba PlanMiner. Solo en local: old/
// no se sube.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "casos.hpp"
#include "conversion.hpp"
#include "inslv/inslv.hpp"
#include "referencias.hpp"

namespace {

class Oraculo : public testing::TestWithParam<std::filesystem::path> {};

TEST_P(Oraculo, MismasReglasYClasesQueOld) {
  const inslv::Datos datos = inslv::pruebas::leer_caso(GetParam()).datos;

  for (const inslv::TipoDeReglas tipo :
       {inslv::TipoDeReglas::descriptivas, inslv::TipoDeReglas::discriminantes}) {
    const std::string nombre =
        tipo == inslv::TipoDeReglas::descriptivas ? "descriptivas" : "discriminantes";
    inslv::Opciones opciones;
    opciones.reglas = tipo;
    const inslv::pruebas::ResultadoDeOld original =
        inslv::pruebas::aprender_con_old(datos, opciones, datos);
    const std::vector<inslv::Regla> reglas = inslv::aprender(datos, opciones);

    EXPECT_EQ(inslv::pruebas::describir(reglas), inslv::pruebas::describir(original.reglas))
        << nombre;
    EXPECT_EQ(inslv::pruebas::describir(inslv::clasificar(reglas, datos)),
              inslv::pruebas::describir(original.clases))
        << nombre;
  }
}

// Lo que necesita un programa que discretiza con rand() después de aprender, como PlanMiner.
TEST_P(Oraculo, ElGeneradorAcabaDondeOldDejaRand) {
  const inslv::Datos datos = inslv::pruebas::leer_caso(GetParam()).datos;
  inslv::Opciones opciones;
  static_cast<void>(inslv::pruebas::aprender_con_old(datos, opciones, datos));
  const int siguiente_de_old = std::rand();  // NOLINT(concurrency-mt-unsafe)

  inslv::GeneradorDeAzar azar;
  opciones.azar = &azar;
  static_cast<void>(inslv::aprender(datos, opciones));
  EXPECT_EQ(azar(), siguiente_de_old);
}

INSTANTIATE_TEST_SUITE_P(PlanMiner, Oraculo, testing::ValuesIn(inslv::pruebas::ficheros_de_casos()),
                         [](const testing::TestParamInfo<std::filesystem::path>& fichero) {
                           std::string nombre = fichero.param.stem().string();
                           std::replace(nombre.begin(), nombre.end(), '-', '_');
                           return nombre;
                         });

}  // namespace
