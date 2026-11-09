// SPDX-License-Identifier: GPL-3.0-only
// Las reglas características: las descriptivas con lo que cumplen casi todos sus ejemplos.

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/aprendizaje/caracteristicas.hpp"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/codigo.hpp"
#include "inslv/inslv.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace {

inslv::Variable logica(const std::string& nombre) {
  return {nombre, {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}};
}

// 40 ejemplos: la clase es a. En los de la clase 1, b es 1 salvo en dos (el ruido) y c va
// alternando. En los de la clase 0, b y c al azar.
inslv::Datos datos_con_ruido() {
  inslv::Datos datos;
  datos.atributos = {logica("a"), logica("b"), logica("c")};
  datos.clase = {"Clase", {{"No", 0, 0, 0, 0}, {"Si", 1, 1, 1, 1}}};
  datos.valores.assign(3, {});
  for (std::size_t e = 0; e < 40; ++e) {
    const std::size_t clase = e % 2;
    datos.valores[0].push_back(static_cast<double>(clase));
    const bool ruido = e == 1 || e == 21;
    auto b = static_cast<double>((e / 2) % 2);
    if (clase == 1) {
      b = ruido ? 0 : 1;
    }
    datos.valores[1].push_back(b);
    datos.valores[2].push_back(static_cast<double>((e / 4) % 2));
    datos.clases.push_back(clase);
  }
  return datos;
}

class Caracteristicas : public testing::Test {
 protected:
  Caracteristicas() {
    reglas.poner_variables(variables);
    // Si a es True, Si: solo la condición que distingue.
    reglas.anadir({"010000", 1, {1, 0, 0, 0.5}}, 0.5);
  }

  std::vector<std::vector<double>> columnas() {
    std::vector<std::vector<double>> resultado = datos.valores;
    std::vector<double> clases;
    clases.reserve(datos.clases.size());
    for (const std::size_t clase : datos.clases) {
      clases.push_back(static_cast<double>(clase));
    }
    resultado.push_back(clases);
    return resultado;
  }

  inslv::Datos datos = datos_con_ruido();
  inslv::difuso::Variables variables{datos};
  inslv::datos::Ejemplos ejemplos{columnas()};
  inslv::reglas::ConjuntoDeReglas reglas;
};

TEST_F(Caracteristicas, AnadeLoQueCumplenCasiTodosSusEjemplos) {
  // b es 1 en 18 de los 20 ejemplos que cubre: con una tolerancia del 10 %, entra.
  const std::vector<inslv::Condicion> condiciones =
      inslv::aprendizaje::caracterizar(reglas, 0, variables, ejemplos, 0.1);
  ASSERT_EQ(condiciones.size(), 2U);
  EXPECT_EQ(condiciones[0].atributo, 0U);
  EXPECT_EQ(condiciones[0].etiquetas, std::vector<std::size_t>{1});
  EXPECT_EQ(condiciones[1].atributo, 1U);
  EXPECT_EQ(condiciones[1].etiquetas, std::vector<std::size_t>{1});
}

TEST_F(Caracteristicas, SinToleranciaSoloLoQueCumplenTodos) {
  const std::vector<inslv::Condicion> condiciones =
      inslv::aprendizaje::caracterizar(reglas, 0, variables, ejemplos, 0);
  ASSERT_EQ(condiciones.size(), 1U);
  EXPECT_EQ(condiciones[0].atributo, 0U);
}

TEST_F(Caracteristicas, NoAnadeLoQueNoDistinguenSusEjemplos) {
  // c es 1 en la mitad: ni con una tolerancia del 40 % entra.
  const std::vector<inslv::Condicion> condiciones =
      inslv::aprendizaje::caracterizar(reglas, 0, variables, ejemplos, 0.4);
  for (const inslv::Condicion& condicion : condiciones) {
    EXPECT_NE(condicion.atributo, 2U);
  }
}

TEST(Aprender, LasCaracteristicasTienenAlMenosLasCondicionesDeLasDescriptivas) {
  const inslv::Datos datos = datos_con_ruido();
  inslv::Opciones opciones;
  const std::vector<inslv::Regla> descriptivas = inslv::aprender(datos, opciones);
  opciones.reglas = inslv::TipoDeReglas::caracteristicas;
  opciones.tolerancia_al_ruido = 0.1;
  const std::vector<inslv::Regla> caracteristicas = inslv::aprender(datos, opciones);

  ASSERT_EQ(caracteristicas.size(), descriptivas.size());
  for (std::size_t r = 0; r < descriptivas.size(); ++r) {
    EXPECT_EQ(caracteristicas[r].consecuente, descriptivas[r].consecuente);
    EXPECT_EQ(caracteristicas[r].peso, descriptivas[r].peso);
    for (const inslv::Condicion& condicion : descriptivas[r].antecedente) {
      bool esta = false;
      for (const inslv::Condicion& otra : caracteristicas[r].antecedente) {
        esta = esta || otra.atributo == condicion.atributo;
      }
      EXPECT_TRUE(esta) << "regla " << r << ", atributo " << condicion.atributo;
    }
  }
}

TEST(Aprender, LaToleranciaTieneQueEstarEntreCeroYUno) {
  const inslv::Datos datos = datos_con_ruido();
  for (const double tolerancia : {-0.1, 1.0, std::numeric_limits<double>::quiet_NaN()}) {
    inslv::Opciones opciones;
    opciones.tolerancia_al_ruido = tolerancia;
    EXPECT_THROW(static_cast<void>(inslv::aprender(datos, opciones)), std::invalid_argument)
        << tolerancia;
  }
}

}  // namespace
