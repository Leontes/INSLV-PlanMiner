// SPDX-License-Identifier: GPL-3.0-only
// Comparación con old/, compilado en el mismo programa (inslv_legacy).
//
// La librería tiene que dar exactamente las mismas reglas que el original, descriptivas y
// discriminantes: antecedente, clase, ejemplos cubiertos y peso, bit a bit. Es más estricto que
// la equivalencia con los casos de PlanMiner, que solo compara lo que interpretaba PlanMiner.
// Solo en local: old/ no se sube.

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <string>

#include "casos.hpp"
#include "conversion.hpp"
#include "inslv/inslv.hpp"
#include "referencias.hpp"

namespace {

class Oraculo : public testing::TestWithParam<std::filesystem::path> {};

TEST_P(Oraculo, MismasReglasQueOld) {
  const inslv::Dataset datos = inslv::pruebas::leer_caso(GetParam()).datos;

  for (const inslv::RuleType tipo : {inslv::RuleType::descriptive, inslv::RuleType::discriminant}) {
    inslv::Options opciones;
    opciones.rules = tipo;
    const std::string original = inslv::pruebas::describir(inslv::pruebas::de_legacy(
        inslv_legacy::learn(inslv::pruebas::a_legacy(datos), inslv::pruebas::a_legacy(opciones))));
    EXPECT_EQ(inslv::pruebas::describir(inslv::learn(datos, opciones)), original)
        << (tipo == inslv::RuleType::descriptive ? "descriptivas" : "discriminantes");
  }
}

INSTANTIATE_TEST_SUITE_P(PlanMiner, Oraculo, testing::ValuesIn(inslv::pruebas::ficheros_de_casos()),
                         [](const testing::TestParamInfo<std::filesystem::path>& fichero) {
                           std::string nombre = fichero.param.stem().string();
                           std::replace(nombre.begin(), nombre.end(), '-', '_');
                           return nombre;
                         });

}  // namespace
