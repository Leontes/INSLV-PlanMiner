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

inslv::Datos pick_up() {
  return inslv::pruebas::leer_caso(fs::path(INSLV_DATOS) / "planminer" /
                                   "blocksworld-PICK_UP-logic.case")
      .datos;
}

// irs, entrenando con las particiones 1 a 9.
inslv::Datos irs() {
  const fs::path directorio = fs::path(INSLV_DATOS) / "nslv" / "irs";
  std::vector<fs::path> particiones;
  for (int i = 1; i < 10; ++i) {
    particiones.push_back(directorio / ("irs" + std::to_string(i) + ".datos"));
  }
  return inslv::nslv::leer(directorio / "irs.dom", particiones);
}

std::vector<inslv::Regla> aprender(const inslv::Datos& datos, inslv::TipoDeReglas tipo) {
  inslv::Opciones opciones;
  opciones.reglas = tipo;
  return inslv::aprender(datos, opciones);
}

// Las etiquetas que admite el atributo en la regla, o nada si la regla no lo usa.
const inslv::Condicion* condicion(const inslv::Regla& regla, std::size_t atributo) {
  const auto encontrada =
      std::find_if(regla.antecedente.begin(), regla.antecedente.end(),
                   [&](const inslv::Condicion& otra) { return otra.atributo == atributo; });
  return encontrada == regla.antecedente.end() ? nullptr : &*encontrada;
}

// Cada discriminante es una generalización de su descriptiva: misma clase y mismo peso, y cada
// condición suya está en la descriptiva con las mismas etiquetas o menos.
void comprueba_generalizacion(const inslv::Datos& datos) {
  const std::vector<inslv::Regla> descriptivas = aprender(datos, inslv::TipoDeReglas::descriptivas);
  const std::vector<inslv::Regla> discriminantes =
      aprender(datos, inslv::TipoDeReglas::discriminantes);

  ASSERT_EQ(discriminantes.size(), descriptivas.size());
  std::size_t mas_cortas = 0;
  for (std::size_t r = 0; r < descriptivas.size(); ++r) {
    const inslv::Regla& especifica = descriptivas[r];
    const inslv::Regla& general = discriminantes[r];
    EXPECT_EQ(general.consecuente, especifica.consecuente) << "regla " << r;
    EXPECT_EQ(general.peso, especifica.peso) << "regla " << r;
    EXPECT_GE(general.cubiertos, especifica.cubiertos) << "regla " << r;
    for (const inslv::Condicion& amplia : general.antecedente) {
      const inslv::Condicion* const estrecha = condicion(especifica, amplia.atributo);
      ASSERT_NE(estrecha, nullptr) << "regla " << r << ", atributo " << amplia.atributo;
      for (const std::size_t etiqueta : estrecha->etiquetas) {
        EXPECT_NE(std::find(amplia.etiquetas.begin(), amplia.etiquetas.end(), etiqueta),
                  amplia.etiquetas.end())
            << "regla " << r << ", atributo " << amplia.atributo << ", etiqueta " << etiqueta;
      }
    }
    if (general.antecedente.size() < especifica.antecedente.size()) {
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
  const std::vector<inslv::Regla> reglas = aprender(pick_up(), inslv::TipoDeReglas::discriminantes);

  ASSERT_EQ(reglas.size(), 2U);
  for (const inslv::Regla& regla : reglas) {
    ASSERT_EQ(regla.antecedente.size(), 1U);
    const inslv::Condicion& unica = regla.antecedente[0];
    EXPECT_EQ(unica.atributo, regla.consecuente == 1 ? 2U : 0U);
    EXPECT_EQ(unica.etiquetas, std::vector<std::size_t>{regla.consecuente == 1 ? 0U : 1U});
  }
}

TEST(Opciones, ElPesoEstaEntreCeroYUno) {
  for (const inslv::TipoDeReglas tipo :
       {inslv::TipoDeReglas::descriptivas, inslv::TipoDeReglas::discriminantes}) {
    for (const inslv::Regla& regla : aprender(irs(), tipo)) {
      EXPECT_GT(regla.peso, 0.0);
      EXPECT_LE(regla.peso, 1.0);
    }
  }
}

TEST(Opciones, EpisodiosPequenos) {
  // Con episodios de 20 ejemplos, irs (135) se aprende en 7.
  inslv::Opciones opciones;
  opciones.tamano_episodio = 20;
  const std::vector<inslv::Regla> reglas = inslv::aprender(irs(), opciones);
  EXPECT_FALSE(reglas.empty());
  EXPECT_EQ(inslv::aprender(irs(), opciones).size(), reglas.size());
}

TEST(Opciones, ElEpisodioTieneQueSerValido) {
  inslv::Opciones opciones;
  opciones.tamano_episodio = 0;
  EXPECT_THROW(inslv::aprender(irs(), opciones), std::invalid_argument);
  opciones.tamano_episodio = static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
  EXPECT_THROW(inslv::aprender(irs(), opciones), std::invalid_argument);
}

}  // namespace
