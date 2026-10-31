// SPDX-License-Identifier: GPL-3.0-only
// Las opciones de learn: el tipo de reglas, su peso y el tamaño del episodio.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "casos.hpp"
#include "inslv/formatos/nslv.hpp"
#include "inslv/inslv.hpp"

namespace {

namespace fs = std::filesystem;

inslv::Dataset pick_up() {
  return inslv::pruebas::leer_caso(fs::path(INSLV_DATOS) / "planminer" /
                                   "blocksworld-PICK_UP-logic.case")
      .datos;
}

// irs, entrenando con las particiones 1 a 9.
inslv::Dataset irs() {
  const fs::path directorio = fs::path(INSLV_DATOS) / "nslv" / "irs";
  std::vector<fs::path> particiones;
  for (int i = 1; i < 10; ++i) {
    particiones.push_back(directorio / ("irs" + std::to_string(i) + ".datos"));
  }
  return inslv::nslv::leer(directorio / "irs.dom", particiones);
}

std::vector<inslv::Rule> aprender(const inslv::Dataset& datos, inslv::RuleType tipo) {
  inslv::Options opciones;
  opciones.rules = tipo;
  return inslv::learn(datos, opciones);
}

// Las etiquetas que admite el atributo en la regla, o nada si la regla no lo usa.
const inslv::Condition* condicion(const inslv::Rule& regla, std::size_t atributo) {
  const auto encontrada =
      std::find_if(regla.antecedent.begin(), regla.antecedent.end(),
                   [&](const inslv::Condition& otra) { return otra.attribute == atributo; });
  return encontrada == regla.antecedent.end() ? nullptr : &*encontrada;
}

// Cada discriminante es una generalización de su descriptiva: misma clase y mismo peso, y cada
// condición suya está en la descriptiva con las mismas etiquetas o menos.
void comprueba_generalizacion(const inslv::Dataset& datos) {
  const std::vector<inslv::Rule> descriptivas = aprender(datos, inslv::RuleType::descriptive);
  const std::vector<inslv::Rule> discriminantes = aprender(datos, inslv::RuleType::discriminant);

  ASSERT_EQ(discriminantes.size(), descriptivas.size());
  std::size_t mas_cortas = 0;
  for (std::size_t r = 0; r < descriptivas.size(); ++r) {
    const inslv::Rule& especifica = descriptivas[r];
    const inslv::Rule& general = discriminantes[r];
    EXPECT_EQ(general.consequent, especifica.consequent) << "regla " << r;
    EXPECT_EQ(general.weight, especifica.weight) << "regla " << r;
    EXPECT_GE(general.covered, especifica.covered) << "regla " << r;
    for (const inslv::Condition& amplia : general.antecedent) {
      const inslv::Condition* const estrecha = condicion(especifica, amplia.attribute);
      ASSERT_NE(estrecha, nullptr) << "regla " << r << ", atributo " << amplia.attribute;
      for (const std::size_t etiqueta : estrecha->labels) {
        EXPECT_NE(std::find(amplia.labels.begin(), amplia.labels.end(), etiqueta),
                  amplia.labels.end())
            << "regla " << r << ", atributo " << amplia.attribute << ", etiqueta " << etiqueta;
      }
    }
    if (general.antecedent.size() < especifica.antecedent.size()) {
      ++mas_cortas;
    }
  }
  EXPECT_GT(mas_cortas, 0U);
}

TEST(Opciones, LasDiscriminantesGeneralizanLasDescriptivas) {
  comprueba_generalizacion(pick_up());
  comprueba_generalizacion(irs());
}

TEST(Opciones, LasDiscriminantesSoloTienenLoMinimo) {
  // PICK_UP: el estado de después (clase 1) es el único sin HANDEMPTY (atributo 2) y el de antes
  // (clase 0), el único con ONTABLE (atributo 0). Las descriptivas tienen los cuatro atributos.
  const std::vector<inslv::Rule> reglas = aprender(pick_up(), inslv::RuleType::discriminant);

  ASSERT_EQ(reglas.size(), 2U);
  for (const inslv::Rule& regla : reglas) {
    ASSERT_EQ(regla.antecedent.size(), 1U);
    const inslv::Condition& unica = regla.antecedent[0];
    EXPECT_EQ(unica.attribute, regla.consequent == 1 ? 2U : 0U);
    EXPECT_EQ(unica.labels, std::vector<std::size_t>{regla.consequent == 1 ? 0U : 1U});
  }
}

TEST(Opciones, ElPesoEstaEntreCeroYUno) {
  for (const inslv::RuleType tipo : {inslv::RuleType::descriptive, inslv::RuleType::discriminant}) {
    for (const inslv::Rule& regla : aprender(irs(), tipo)) {
      EXPECT_GT(regla.weight, 0.0);
      EXPECT_LE(regla.weight, 1.0);
    }
  }
}

TEST(Opciones, EpisodiosPequenos) {
  // Con episodios de 20 ejemplos, irs (135) se aprende en 7.
  inslv::Options opciones;
  opciones.episode_size = 20;
  const std::vector<inslv::Rule> reglas = inslv::learn(irs(), opciones);
  EXPECT_FALSE(reglas.empty());
  EXPECT_EQ(inslv::learn(irs(), opciones).size(), reglas.size());
}

TEST(Opciones, ElEpisodioTieneQueSerValido) {
  inslv::Options opciones;
  opciones.episode_size = 0;
  EXPECT_THROW(inslv::learn(irs(), opciones), std::invalid_argument);
  opciones.episode_size = static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
  EXPECT_THROW(inslv::learn(irs(), opciones), std::invalid_argument);
}

}  // namespace
