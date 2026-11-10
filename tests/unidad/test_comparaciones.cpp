// SPDX-License-Identifier: GPL-3.0-only
// Las comparaciones entre atributos de la misma unidad (Opciones::comparaciones).

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "inslv/aprendizaje/comparaciones.hpp"
#include "inslv/inslv.hpp"

namespace {

using inslv::aprendizaje::Par;

constexpr double nan = std::numeric_limits<double>::quiet_NaN();

// Un atributo en [0, 10] con cinco etiquetas triangulares.
inslv::Variable medida(const std::string& nombre, unsigned unidad) {
  inslv::Variable variable{nombre,
                           {{"MuyBaja", 0, 0, 0, 2.5},
                            {"Baja", 0, 2.5, 2.5, 5},
                            {"Media", 2.5, 5, 5, 7.5},
                            {"Alta", 5, 7.5, 7.5, 10},
                            {"MuyAlta", 7.5, 10, 10, 10}}};
  variable.unidad = unidad;
  return variable;
}

inslv::Variable dos_clases() { return {"Clase", {{"No", 0, 0, 0, 0}, {"Si", 1, 1, 1, 1}}}; }

// 100 ejemplos, x e y de 0,5 a 9,5, en la misma unidad: la clase es Si si x < y. Con etiquetas de
// un solo atributo no se puede decir; comparándolos, sí. constante vale siempre lo mismo que
// otra_constante (las dos en otra unidad).
inslv::Datos x_menor_que_y() {
  inslv::Datos datos;
  datos.atributos = {medida("x", 1), medida("y", 1), medida("constante", 2),
                     medida("otra_constante", 2)};
  datos.clase = dos_clases();
  datos.valores.assign(4, {});
  for (std::size_t i = 0; i < 10; ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      const double x = 0.5 + static_cast<double>(i);
      const double y = 0.5 + static_cast<double>(j);
      datos.valores[0].push_back(x);
      datos.valores[1].push_back(y);
      datos.valores[2].push_back(5);
      datos.valores[3].push_back(5);
      datos.clases.push_back(x < y ? 1 : 0);
    }
  }
  return datos;
}

std::size_t aciertos(const std::vector<inslv::Regla>& reglas, const inslv::Datos& datos) {
  const std::vector<std::optional<std::size_t>> clases = inslv::clasificar(reglas, datos);
  std::size_t bien = 0;
  for (std::size_t e = 0; e < clases.size(); ++e) {
    bien += clases[e] == datos.clases[e] ? 1U : 0U;
  }
  return bien;
}

bool compara(const std::vector<inslv::Regla>& reglas) {
  for (const inslv::Regla& regla : reglas) {
    for (const inslv::Condicion& condicion : regla.antecedente) {
      if (condicion.comparado_con) {
        return true;
      }
    }
  }
  return false;
}

TEST(Comparaciones, SoloLosParesDeLaMismaUnidad) {
  inslv::Datos datos;
  for (const unsigned unidad : {1U, 0U, 1U, 2U, 1U, 2U, 0U}) {
    datos.atributos.push_back(medida("v", unidad));
  }
  const std::vector<Par> pares = inslv::aprendizaje::pares_de_la_misma_unidad(datos);
  const std::vector<std::pair<std::size_t, std::size_t>> esperados = {
      {0, 2}, {0, 4}, {2, 4}, {3, 5}};
  ASSERT_EQ(pares.size(), esperados.size());
  for (std::size_t i = 0; i < pares.size(); ++i) {
    EXPECT_EQ(pares[i].primero, esperados[i].first);
    EXPECT_EQ(pares[i].segundo, esperados[i].second);
  }
}

TEST(Comparaciones, ElResultadoDeCompararDosAtributos) {
  inslv::Datos datos;
  datos.atributos = {medida("x", 1), medida("y", 1)};
  datos.valores = {{1, 2, 3, nan, 4}, {2, 2, 1, 0, nan}};
  const std::vector<double> valores = inslv::aprendizaje::valores_de_comparacion(datos, {0, 1});
  ASSERT_EQ(valores.size(), 5U);
  EXPECT_EQ(valores[0], static_cast<double>(inslv::comparacion::menor));
  EXPECT_EQ(valores[1], static_cast<double>(inslv::comparacion::igual));
  EXPECT_EQ(valores[2], static_cast<double>(inslv::comparacion::mayor));
  EXPECT_TRUE(std::isnan(valores[3]));
  EXPECT_TRUE(std::isnan(valores[4]));

  const inslv::Variable variable = inslv::aprendizaje::variable_de_comparacion(datos, {0, 1});
  EXPECT_EQ(variable.nombre, "x frente a y");
  ASSERT_EQ(variable.etiquetas.size(), 3U);
  EXPECT_EQ(variable.etiquetas[inslv::comparacion::menor].etiqueta, "menor");
  EXPECT_EQ(variable.etiquetas[inslv::comparacion::igual].etiqueta, "igual");
  EXPECT_EQ(variable.etiquetas[inslv::comparacion::mayor].etiqueta, "mayor");
}

TEST(Comparaciones, SeQuedanLasQueDanInformacion) {
  const inslv::Datos datos = x_menor_que_y();
  const std::vector<Par> pares = inslv::aprendizaje::pares_de_la_misma_unidad(datos);
  ASSERT_EQ(pares.size(), 2U);
  // Las dos constantes siempre son iguales: su comparación no dice nada de la clase.
  const std::vector<Par> elegidas = inslv::aprendizaje::elegir_comparaciones(datos, pares, 0.2);
  ASSERT_EQ(elegidas.size(), 1U);
  EXPECT_EQ(elegidas[0].primero, 0U);
  EXPECT_EQ(elegidas[0].segundo, 1U);
}

TEST(Comparaciones, LosDatosConLasComparaciones) {
  const inslv::Datos datos = x_menor_que_y();
  const inslv::Datos con = inslv::aprendizaje::con_comparaciones(datos, {{0, 1}});
  ASSERT_EQ(con.atributos.size(), 5U);
  ASSERT_EQ(con.valores.size(), 5U);
  EXPECT_EQ(con.atributos[4].nombre, "x frente a y");
  EXPECT_EQ(con.valores[4], inslv::aprendizaje::valores_de_comparacion(datos, {0, 1}));
  EXPECT_EQ(con.clases, datos.clases);
}

TEST(Comparaciones, TraduceLasCondicionesSobreLasComparaciones) {
  inslv::Regla regla;
  regla.antecedente = {{0, {1, 2}}, {5, {inslv::comparacion::menor}}};
  std::vector<inslv::Regla> reglas = {regla};
  inslv::aprendizaje::traducir_comparaciones(reglas, 4, {{0, 1}, {2, 3}});
  const std::vector<inslv::Condicion>& antecedente = reglas[0].antecedente;
  EXPECT_EQ(antecedente[0].atributo, 0U);
  EXPECT_FALSE(antecedente[0].comparado_con);
  EXPECT_EQ(antecedente[1].atributo, 2U);
  EXPECT_EQ(antecedente[1].comparado_con, 3U);
  EXPECT_EQ(antecedente[1].etiquetas, (std::vector<std::size_t>{inslv::comparacion::menor}));
}

TEST(Aprender, ConComparacionesAprendeUnaRelacion) {
  const inslv::Datos datos = x_menor_que_y();
  inslv::Opciones opciones;
  opciones.comparaciones = true;
  const std::vector<inslv::Regla> reglas = inslv::aprender(datos, opciones);
  EXPECT_TRUE(compara(reglas));
  EXPECT_EQ(aciertos(reglas, datos), datos.clases.size());
  for (const inslv::Regla& regla : reglas) {
    for (const inslv::Condicion& condicion : regla.antecedente) {
      if (condicion.comparado_con) {
        EXPECT_EQ(condicion.atributo, 0U);
        EXPECT_EQ(condicion.comparado_con, 1U);
      }
    }
  }
}

TEST(Aprender, SinComparacionesLasReglasNoComparan) {
  const inslv::Datos datos = x_menor_que_y();
  const std::vector<inslv::Regla> reglas = inslv::aprender(datos);
  EXPECT_FALSE(compara(reglas));
  // Sin comparar, lo memoriza casilla a casilla; comparando, basta una regla por clase.
  inslv::Opciones opciones;
  opciones.comparaciones = true;
  EXPECT_EQ(inslv::aprender(datos, opciones).size(), 2U);
  EXPECT_GT(reglas.size(), 2U);
}

TEST(Aprender, ElUmbralDeLasComparacionesNoPuedeSerNegativo) {
  const inslv::Datos datos = x_menor_que_y();
  for (const double umbral : {-0.1, nan}) {
    inslv::Opciones opciones;
    opciones.comparaciones = true;
    opciones.umbral_de_comparaciones = umbral;
    EXPECT_THROW(static_cast<void>(inslv::aprender(datos, opciones)), std::invalid_argument)
        << umbral;
  }
}

// Una regla con una comparación entre x (0) e y (1).
inslv::Regla compara_x_con_y(std::vector<std::size_t> resultados, std::size_t clase) {
  inslv::Regla regla;
  regla.antecedente = {{0, std::move(resultados), 1}};
  regla.consecuente = clase;
  regla.peso = 1;
  return regla;
}

inslv::Datos con_valores(const std::vector<double>& x, const std::vector<double>& y) {
  inslv::Datos datos;
  datos.atributos = {medida("x", 1), medida("y", 1)};
  datos.clase = dos_clases();
  datos.valores = {x, y};
  return datos;
}

TEST(Clasificar, UnaComparacionSeCumpleONo) {
  using Clases = std::vector<std::optional<std::size_t>>;
  const inslv::Datos datos = con_valores({1, 2, 3}, {2, 2, 1});
  EXPECT_EQ(inslv::clasificar({compara_x_con_y({inslv::comparacion::menor}, 1)}, datos),
            (Clases{1, std::nullopt, std::nullopt}));
  EXPECT_EQ(
      inslv::clasificar(
          {compara_x_con_y({inslv::comparacion::menor, inslv::comparacion::igual}, 1)}, datos),
      (Clases{1, 1, std::nullopt}));
  EXPECT_EQ(inslv::clasificar({compara_x_con_y({inslv::comparacion::mayor}, 0)}, datos),
            (Clases{std::nullopt, std::nullopt, 0}));
}

TEST(Clasificar, UnaComparacionConUnDesconocidoSeCumple) {
  const inslv::Datos datos = con_valores({nan, 3}, {2, nan});
  EXPECT_EQ(inslv::clasificar({compara_x_con_y({inslv::comparacion::menor}, 1)}, datos),
            (std::vector<std::optional<std::size_t>>{1, 1}));
}

TEST(Clasificar, RechazaComparacionesQueNoExisten) {
  const inslv::Datos datos = con_valores({1}, {2});
  inslv::Regla fuera = compara_x_con_y({inslv::comparacion::menor}, 1);
  fuera.antecedente[0].comparado_con = 2;
  EXPECT_THROW(static_cast<void>(inslv::clasificar({fuera}, datos)), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(inslv::clasificar({compara_x_con_y({3}, 1)}, datos)),
               std::invalid_argument);
}

}  // namespace
