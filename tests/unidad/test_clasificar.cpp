// SPDX-License-Identifier: GPL-3.0-only
// inslv::clasificar con reglas hechas a mano. Que da las mismas clases que la inferencia del
// original lo comprueban las reglas de referencia (equivalencia) y el oráculo.

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

#include "inslv/inslv.hpp"

namespace {

using Clases = std::vector<std::optional<std::size_t>>;

// Un atributo x con tres etiquetas en [0, 10] (Baja, Media y Alta) y dos clases.
inslv::Datos con_valores(const std::vector<double>& valores) {
  inslv::Datos datos;
  datos.atributos = {
      {"x", {{"Baja", 0, 0, 0, 5}, {"Media", 0, 5, 5, 10}, {"Alta", 5, 10, 10, 10}}}};
  datos.clase = {"Clase", {{"No", 0, 0, 0, 0}, {"Si", 1, 1, 1, 1}}};
  datos.valores = {valores};
  return datos;
}

inslv::Regla regla(std::vector<std::size_t> etiquetas, std::size_t clase, double peso) {
  inslv::Regla resultado;
  resultado.antecedente = {{0, std::move(etiquetas)}};
  resultado.consecuente = clase;
  resultado.peso = peso;
  return resultado;
}

TEST(Clasificar, GanaLaMayorAdaptacionPorPeso) {
  // En 0, Baja 1; en 2.5, Baja 0.5 y Alta 0; en 10, Alta 1.
  const std::vector<inslv::Regla> reglas = {regla({0}, 0, 1.0), regla({2}, 1, 1.0)};
  EXPECT_EQ(inslv::clasificar(reglas, con_valores({0, 2.5, 10})), (Clases{0, 0, 1}));
}

// Una regla sin condiciones: se adapta por completo a todo.
inslv::Regla siempre(std::size_t clase, double peso) {
  inslv::Regla resultado;
  resultado.consecuente = clase;
  resultado.peso = peso;
  return resultado;
}

TEST(Clasificar, ConEmpateGanaLaDeMasPesoYSiNoLaPrimera) {
  // En 2.5, siempre(0, 0.5) da 1 * 0.5 y Baja con peso 1 da 0.5 * 1: empatan.
  EXPECT_EQ(inslv::clasificar({siempre(0, 0.5), regla({0}, 1, 1.0)}, con_valores({2.5})),
            (Clases{1}));
  EXPECT_EQ(inslv::clasificar({regla({0}, 1, 1.0), siempre(0, 0.5)}, con_valores({2.5})),
            (Clases{1}));
  // Mismo grado y mismo peso: la primera.
  EXPECT_EQ(inslv::clasificar({regla({0}, 0, 1.0), regla({1}, 1, 1.0)}, con_valores({2.5})),
            (Clases{0}));
  // Sin empate, el mayor grado, aunque la otra tenga más peso.
  EXPECT_EQ(inslv::clasificar({regla({0}, 0, 0.5), regla({1}, 1, 1.0)}, con_valores({2.5})),
            (Clases{1}));
}

TEST(Clasificar, UnValorEntreDosEtiquetasDeLaCondicionSeAdaptaPorCompleto) {
  // En 2.5, Baja 0.5 y Media 0.5. Como el original, que supone una partición: si el valor está en
  // dos etiquetas de la condición, su adaptación es 1, no la mayor (0.5).
  EXPECT_EQ(inslv::clasificar({siempre(0, 0.9), regla({0, 1}, 1, 1.0)}, con_valores({2.5})),
            (Clases{1}));
  // Si solo está en una, la de esa etiqueta: 0.5 * 1 < 0.9.
  EXPECT_EQ(inslv::clasificar({siempre(0, 0.9), regla({0, 2}, 1, 1.0)}, con_valores({2.5})),
            (Clases{0}));
  // El orden de las etiquetas no importa.
  EXPECT_EQ(inslv::clasificar({siempre(0, 0.9), regla({1, 0}, 1, 1.0)}, con_valores({2.5})),
            (Clases{1}));
}

TEST(Clasificar, SinReglaQueDispareNoHayClase) {
  EXPECT_EQ(inslv::clasificar({regla({0}, 0, 1.0)}, con_valores({10, 0})),
            (Clases{std::nullopt, 0}));
  EXPECT_EQ(inslv::clasificar({regla({}, 0, 1.0)}, con_valores({5})), (Clases{std::nullopt}));
  EXPECT_EQ(inslv::clasificar({regla({0}, 0, 0.0)}, con_valores({0})), (Clases{std::nullopt}));
  EXPECT_EQ(inslv::clasificar({}, con_valores({0})), (Clases{std::nullopt}));
}

TEST(Clasificar, UnDesconocidoSeAdaptaATodo) {
  EXPECT_EQ(inslv::clasificar({regla({2}, 1, 1.0)},
                              con_valores({std::numeric_limits<double>::quiet_NaN()})),
            (Clases{1}));
}

TEST(Clasificar, LaReglaSinCondicionesDisparaSiempre) {
  EXPECT_EQ(inslv::clasificar({regla({0}, 0, 1.0), siempre(1, 0.25)}, con_valores({0, 10})),
            (Clases{0, 1}));
}

TEST(Clasificar, SinEjemplosNoHayClases) {
  EXPECT_TRUE(inslv::clasificar({regla({0}, 0, 1.0)}, con_valores({})).empty());
}

TEST(Clasificar, RechazaReglasYDatosIncoherentes) {
  inslv::Regla fuera = regla({0}, 0, 1.0);
  fuera.antecedente[0].atributo = 1;
  EXPECT_THROW(inslv::clasificar({fuera}, con_valores({0})), std::invalid_argument);
  EXPECT_THROW(inslv::clasificar({regla({3}, 0, 1.0)}, con_valores({0})), std::invalid_argument);

  inslv::Datos datos = con_valores({0});
  datos.valores.clear();
  EXPECT_THROW(inslv::clasificar({}, datos), std::invalid_argument);
}

}  // namespace
