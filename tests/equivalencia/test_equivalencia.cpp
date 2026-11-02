// SPDX-License-Identifier: GPL-3.0-only
// Equivalencia con la implementación original.
//
// Cada caso de tests/datos/planminer es una llamada a inslv() del PlanMiner original: la entrada
// y las reglas que devolvió, ya interpretadas por PlanMiner. learn() tiene que dar las mismas
// reglas con la misma interpretación. No necesita old/: corre también en la CI.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "casos.hpp"
#include "inslv/inslv.hpp"

namespace {

using inslv::pruebas::ReglaPlanMiner;

// La interpretación de PlanMiner (ruleset::SplitAntecedent y ruleset::toPDDL del original): solo
// cuentan los atributos de hasta dos etiquetas; el valor es el nombre de la etiqueta si es True o
// False y, si no, su centro truncado a entero. La clase va en el signo.
ReglaPlanMiner interpretar(const inslv::Regla& regla, const inslv::Datos& datos) {
  ReglaPlanMiner resultado;
  const bool post_state = datos.clase.etiquetas[regla.consecuente].etiqueta == "PostState";
  resultado.cubiertos =
      post_state ? static_cast<int>(regla.cubiertos) : static_cast<int>(0 - regla.cubiertos);
  for (const inslv::Condicion& condicion : regla.antecedente) {
    const inslv::Variable& atributo = datos.atributos[condicion.atributo];
    if (atributo.etiquetas.size() > 2) {
      continue;
    }
    for (const std::size_t etiqueta : condicion.etiquetas) {
      const inslv::ConjuntoDifuso& conjunto = atributo.etiquetas[etiqueta];
      if (conjunto.etiqueta == "True" || conjunto.etiqueta == "False") {
        resultado.condiciones.emplace_back(atributo.nombre, conjunto.etiqueta);
      } else {
        resultado.condiciones.emplace_back(
            atributo.nombre, std::to_string(static_cast<int>((conjunto.b + conjunto.c) / 2.0)));
      }
    }
  }
  return resultado;
}

class Equivalencia : public testing::TestWithParam<std::filesystem::path> {};

TEST_P(Equivalencia, MismasReglasQueElOriginal) {
  const inslv::pruebas::CasoPlanMiner esperado = inslv::pruebas::leer_caso(GetParam());

  std::vector<ReglaPlanMiner> aprendidas;
  for (const inslv::Regla& regla : inslv::aprender(esperado.datos)) {
    aprendidas.push_back(interpretar(regla, esperado.datos));
  }

  ASSERT_EQ(aprendidas.size(), esperado.reglas.size());
  for (std::size_t r = 0; r < aprendidas.size(); ++r) {
    EXPECT_EQ(aprendidas[r], esperado.reglas[r]) << "regla " << r;
  }
}

INSTANTIATE_TEST_SUITE_P(PlanMiner, Equivalencia,
                         testing::ValuesIn(inslv::pruebas::ficheros_de_casos()),
                         [](const testing::TestParamInfo<std::filesystem::path>& fichero) {
                           std::string nombre = fichero.param.stem().string();
                           std::replace(nombre.begin(), nombre.end(), '-', '_');
                           return nombre;
                         });

}  // namespace
