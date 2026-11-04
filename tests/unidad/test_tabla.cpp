// SPDX-License-Identifier: GPL-3.0-only
// La tabla de adaptaciones y las medidas de información, con cuentas hechas a mano.

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/aprendizaje/medidas.h"
#include "inslv/aprendizaje/tabla.h"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/inslv.hpp"

namespace {

inslv::Variable logica(const std::string& nombre) {
  return {nombre, {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}};
}

// t: dos etiquetas que se solapan; c: una lógica; y la clase.
inslv::difuso::Variables variables() {
  inslv::Datos datos;
  datos.atributos = {{"t", {{"Bajo", 0, 0, 0, 1}, {"Alto", 0, 1, 1, 1}}}, logica("c")};
  datos.clase = logica("Clase");
  return inslv::difuso::Variables(datos);
}

TEST(Tabla, LaAdaptacionDeCadaEtiqueta) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25}, {1}, {0}});
  const inslv::TablaDeAdaptaciones tabla(V, E);
  EXPECT_DOUBLE_EQ(tabla.Etiqueta(0, 0, 0), 0.75);
  EXPECT_DOUBLE_EQ(tabla.Etiqueta(0, 0, 1), 0.25);
  EXPECT_EQ(tabla.Etiqueta(0, 1, 0), 0);
  EXPECT_EQ(tabla.Etiqueta(0, 1, 1), 1);
}

TEST(Tabla, LaAdaptacionAVariasEtiquetas) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25}, {1}, {0}});
  const inslv::TablaDeAdaptaciones tabla(V, E);
  EXPECT_DOUBLE_EQ(tabla.Variable(0, 0, "10"), 0.75);
  EXPECT_DOUBLE_EQ(tabla.Variable(0, 0, "01"), 0.25);
  EXPECT_EQ(tabla.Variable(0, 0, "00"), 0);
  // Las etiquetas forman una partición: si dos admitidas se adaptan, 1.
  EXPECT_EQ(tabla.Variable(0, 0, "11"), 1);
}

TEST(Tabla, ElAntecedenteEsElMenorDeLosAtributosRelevantes) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25}, {1}, {0}});
  const inslv::TablaDeAdaptaciones tabla(V, E);
  // Los dos relevantes: t en Alto (0,25) y c en True (1).
  EXPECT_DOUBLE_EQ(tabla.Antecedente(0, "0101", {1, 1, 0.5}), 0.25);
  // t por debajo del umbral: no cuenta.
  EXPECT_EQ(tabla.Antecedente(0, "0101", {0, 1, 0.5}), 1);
  // c en False, que no se adapta.
  EXPECT_EQ(tabla.Antecedente(0, "0110", {1, 1, 0.5}), 0);
}

TEST(Tabla, ElConsecuenteEsPositivoONegativo) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25}, {1}, {0}});
  const inslv::TablaDeAdaptaciones tabla(V, E);
  double pos = -1;
  double neg = -1;
  tabla.Consecuente(0, 0, pos, neg);
  EXPECT_EQ(pos, 1);
  EXPECT_EQ(neg, 0);
  tabla.Consecuente(0, 1, pos, neg);
  EXPECT_EQ(pos, 0);
  EXPECT_EQ(neg, 1);
}

TEST(Tabla, LaMejorEtiqueta) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25, 0.75}, {1, 1}, {0, 0}});
  const inslv::TablaDeAdaptaciones tabla(V, E);
  EXPECT_EQ(tabla.MejorEtiqueta(0, 0), 0);
  EXPECT_EQ(tabla.MejorEtiqueta(1, 0), 1);
  EXPECT_EQ(tabla.MejorEtiqueta(0, 1), 1);
}

TEST(Tabla, UnValorQueNoSeAdaptaEsUnError) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{5}, {1}, {0}});
  EXPECT_THROW(const inslv::TablaDeAdaptaciones tabla(V, E), std::invalid_argument);
}

TEST(Medidas, InformacionMutuaNormalizada) {
  inslv::Datos datos;
  datos.atributos = {logica("decide"), logica("ruido")};
  datos.clase = logica("Clase");
  const inslv::difuso::Variables V(datos);
  // decide es la clase; ruido es independiente de ella.
  const inslv::datos::Ejemplos E({{0, 0, 1, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}});

  const std::vector<std::vector<double>> I = inslv::MedidasDeInformacion(V, E);

  ASSERT_EQ(I.size(), 2U);
  ASSERT_EQ(I[0].size(), 3U);
  EXPECT_NEAR(I[0][0], 1, 1e-12);
  EXPECT_NEAR(I[0][1], 1, 1e-12);
  EXPECT_NEAR(I[0][2], 1, 1e-12);
  EXPECT_NEAR(std::abs(I[1][0]), 0, 1e-12);
  EXPECT_NEAR(std::abs(I[1][1]), 0, 1e-12);
  EXPECT_NEAR(std::abs(I[1][2]), 0, 1e-12);
}

}  // namespace
