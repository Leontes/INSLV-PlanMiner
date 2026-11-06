// SPDX-License-Identifier: GPL-3.0-only
// La tabla de adaptaciones y las medidas de información, con cuentas hechas a mano.

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/aprendizaje/medidas.hpp"
#include "inslv/aprendizaje/tabla.hpp"
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
  const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E);
  EXPECT_DOUBLE_EQ(tabla.adaptacion_a_etiqueta(0, 0, 0), 0.75);
  EXPECT_DOUBLE_EQ(tabla.adaptacion_a_etiqueta(0, 0, 1), 0.25);
  EXPECT_EQ(tabla.adaptacion_a_etiqueta(0, 1, 0), 0);
  EXPECT_EQ(tabla.adaptacion_a_etiqueta(0, 1, 1), 1);
}

TEST(Tabla, LaAdaptacionAVariasEtiquetas) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25}, {1}, {0}});
  const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E);
  EXPECT_DOUBLE_EQ(tabla.adaptaciones_a_variable(0, "10")[0], 0.75);
  EXPECT_DOUBLE_EQ(tabla.adaptaciones_a_variable(0, "01")[0], 0.25);
  EXPECT_EQ(tabla.adaptaciones_a_variable(0, "00")[0], 0);
  // Las etiquetas forman una partición: si dos admitidas se adaptan, 1.
  EXPECT_EQ(tabla.adaptaciones_a_variable(0, "11")[0], 1);
}

TEST(Tabla, ElAntecedenteEsElMenorDeLosAtributosRelevantes) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25}, {1}, {0}});
  const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E);
  std::vector<double> adaptacion;
  // Los dos relevantes: t en Alto (0,25) y c en True (1).
  tabla.adaptaciones_al_antecedente("0101", {1, 1, 0.5}, adaptacion);
  EXPECT_EQ(adaptacion.size(), 1U);
  EXPECT_DOUBLE_EQ(adaptacion[0], 0.25);
  // t por debajo del umbral: no cuenta.
  tabla.adaptaciones_al_antecedente("0101", {0, 1, 0.5}, adaptacion);
  EXPECT_EQ(adaptacion[0], 1);
  // c en False, que no se adapta.
  tabla.adaptaciones_al_antecedente("0110", {1, 1, 0.5}, adaptacion);
  EXPECT_EQ(adaptacion[0], 0);
  // Ninguno relevante: se adapta del todo.
  tabla.adaptaciones_al_antecedente("0000", {0, 0, 0.5}, adaptacion);
  EXPECT_EQ(adaptacion[0], 1);
}

TEST(Tabla, CadaEjemploConSuAdaptacion) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25, 0.75, 0}, {1, 0, 1}, {0, 1, 0}});
  const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E);
  std::vector<double> adaptacion;
  tabla.adaptaciones_al_antecedente("1001", {1, 1, 0.5}, adaptacion);
  ASSERT_EQ(adaptacion.size(), 3U);
  EXPECT_DOUBLE_EQ(adaptacion[0], 0.75);
  EXPECT_EQ(adaptacion[1], 0);
  EXPECT_EQ(adaptacion[2], 1);
}

TEST(Tabla, OlvidarLosConjuntosNoCambiaElResultado) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25, 0.75, 0}, {1, 0, 1}, {0, 1, 0}});
  const inslv::aprendizaje::TablaDeAdaptaciones con_memoria(V, E);
  // Sin memoria: los olvida en cada llamada.
  const inslv::aprendizaje::TablaDeAdaptaciones sin_memoria(V, E, 0);
  std::vector<double> una;
  std::vector<double> otra;
  for (const std::string etiquetas : {"1001", "0110", "1111", "1001", "0101", "1001"}) {
    con_memoria.adaptaciones_al_antecedente(etiquetas, {1, 1, 0.5}, una);
    sin_memoria.adaptaciones_al_antecedente(etiquetas, {1, 1, 0.5}, otra);
    EXPECT_EQ(una, otra) << etiquetas;
  }
}

TEST(Tabla, LaAdaptacionALaClase) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25, 0.25}, {1, 1}, {0, 1}});
  const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E);
  EXPECT_EQ(tabla.adaptaciones_a_clase(0), (std::vector<double>{1, 0}));
  EXPECT_EQ(tabla.adaptaciones_a_clase(1), (std::vector<double>{0, 1}));
}

TEST(Tabla, LaMejorEtiqueta) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{0.25, 0.75}, {1, 1}, {0, 0}});
  const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E);
  EXPECT_EQ(tabla.mejor_etiqueta(0, 0), 0);
  EXPECT_EQ(tabla.mejor_etiqueta(1, 0), 1);
  EXPECT_EQ(tabla.mejor_etiqueta(0, 1), 1);
}

TEST(Tabla, UnValorQueNoSeAdaptaEsUnError) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E({{5}, {1}, {0}});
  EXPECT_THROW(const inslv::aprendizaje::TablaDeAdaptaciones tabla(V, E), std::invalid_argument);
}

TEST(Medidas, InformacionMutuaNormalizada) {
  inslv::Datos datos;
  datos.atributos = {logica("decide"), logica("ruido")};
  datos.clase = logica("Clase");
  const inslv::difuso::Variables V(datos);
  // decide es la clase; ruido es independiente de ella.
  const inslv::datos::Ejemplos E({{0, 0, 1, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}});

  const std::vector<std::vector<double>> I = inslv::aprendizaje::medidas_de_informacion(V, E);

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
