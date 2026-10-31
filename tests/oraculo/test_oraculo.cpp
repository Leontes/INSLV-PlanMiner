// SPDX-License-Identifier: GPL-3.0-only
// Comparación con old/, compilado en el mismo programa (inslv_legacy).
//
// La librería tiene que dar exactamente las mismas reglas que el original: antecedente, clase y
// ejemplos cubiertos, bit a bit. Es más estricto que la equivalencia, que solo compara lo que
// interpretaba PlanMiner. Solo en local: old/ no se sube.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include "casos.hpp"
#include "inslv/inslv.hpp"
#include "inslv_legacy.hpp"

namespace {

inslv_legacy::Variable a_legacy(const inslv::Variable& variable) {
  inslv_legacy::Variable resultado;
  resultado.name = variable.name;
  for (const inslv::FuzzySet& etiqueta : variable.labels) {
    resultado.labels.push_back({etiqueta.label, etiqueta.a, etiqueta.b, etiqueta.c, etiqueta.d});
  }
  return resultado;
}

inslv_legacy::Dataset a_legacy(const inslv::Dataset& datos) {
  inslv_legacy::Dataset resultado;
  for (const inslv::Variable& atributo : datos.attributes) {
    resultado.attributes.push_back(a_legacy(atributo));
  }
  resultado.target = a_legacy(datos.target);
  resultado.values = datos.values;
  resultado.classes = datos.classes;
  return resultado;
}

// Las reglas como texto, para compararlas y ver las diferencias. Los cubiertos van en
// hexadecimal: se comparan bit a bit.
template <typename Regla>
std::string describir(const std::vector<Regla>& reglas) {
  std::ostringstream salida;
  salida << std::hexfloat;
  for (const Regla& regla : reglas) {
    salida << "clase " << regla.consequent << " (" << regla.covered << "):";
    for (const auto& condicion : regla.antecedent) {
      salida << " atributo " << condicion.attribute << " en {";
      for (const std::size_t etiqueta : condicion.labels) {
        salida << ' ' << etiqueta;
      }
      salida << " }";
    }
    salida << '\n';
  }
  return salida.str();
}

class Oraculo : public testing::TestWithParam<std::filesystem::path> {};

TEST_P(Oraculo, MismasReglasQueOld) {
  const inslv::Dataset datos = inslv::pruebas::leer_caso(GetParam()).datos;

  const std::string original = describir(inslv_legacy::learn(a_legacy(datos)));
  ASSERT_FALSE(original.empty());
  EXPECT_EQ(describir(inslv::learn(datos)), original);
}

INSTANTIATE_TEST_SUITE_P(PlanMiner, Oraculo, testing::ValuesIn(inslv::pruebas::ficheros_de_casos()),
                         [](const testing::TestParamInfo<std::filesystem::path>& fichero) {
                           std::string nombre = fichero.param.stem().string();
                           std::replace(nombre.begin(), nombre.end(), '-', '_');
                           return nombre;
                         });

}  // namespace
