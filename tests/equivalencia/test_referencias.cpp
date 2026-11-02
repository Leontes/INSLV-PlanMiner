// SPDX-License-Identifier: GPL-3.0-only
// Equivalencia con las reglas de referencia (tests/datos/referencias): las que dio old/ con los
// conjuntos de NSLV, con reglas descriptivas y discriminantes y con distintos tamaños de
// episodio, y las clases que da su inferencia a la partición de test. aprender tiene que dar las
// mismas reglas, bit a bit, y clasificar, las mismas clases. No necesita old/.
//
// rapidas/ va con la etiqueta equivalencia; lentas/ (aut, boxA, ring), con la etiqueta lento.

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "inslv/inslv.hpp"
#include "referencias.hpp"

namespace {

class Referencias : public testing::TestWithParam<std::filesystem::path> {};

TEST_P(Referencias, MismasReglasYClasesQueOld) {
  const inslv::pruebas::Experimento experimento = inslv::pruebas::experimento_de(GetParam());
  const std::vector<inslv::Regla> reglas =
      inslv::aprender(experimento.entrenamiento(), experimento.opciones());

  EXPECT_EQ(inslv::pruebas::describir(reglas),
            inslv::pruebas::describir(inslv::pruebas::leer_reglas(GetParam())));
  EXPECT_EQ(inslv::pruebas::describir(inslv::clasificar(reglas, experimento.prueba())),
            inslv::pruebas::describir(inslv::pruebas::leer_predicciones(GetParam())));
}

std::string nombre(const testing::TestParamInfo<std::filesystem::path>& fichero) {
  std::string resultado = fichero.param.stem().string();
  std::replace(resultado.begin(), resultado.end(), '-', '_');
  return resultado;
}

INSTANTIATE_TEST_SUITE_P(Rapidas, Referencias,
                         testing::ValuesIn(inslv::pruebas::ficheros_de_referencia("rapidas")),
                         nombre);
INSTANTIATE_TEST_SUITE_P(Lentas, Referencias,
                         testing::ValuesIn(inslv::pruebas::ficheros_de_referencia("lentas")),
                         nombre);

}  // namespace
