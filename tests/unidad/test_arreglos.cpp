// SPDX-License-Identifier: GPL-3.0-only
// Los arreglos de la S4: errores que terminaban el programa, la regla -1 de R_mix y las fugas de
// memoria (las comprueba el preset asan, sin supresiones). La salida del progreso, en
// test_verbosidad.cpp.

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/aprendizaje/RuleCreator.hpp"
#include "inslv/inslv.hpp"

namespace {

inslv::Variable logica(const std::string& nombre) {
  return {nombre, {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}};
}

// 20 ejemplos: la clase es el valor de a.
inslv::Datos clase_es_a() {
  inslv::Datos datos;
  datos.atributos = {logica("a"), logica("b")};
  datos.clase = {"Clase", {{"No", 0, 0, 0, 0}, {"Si", 1, 1, 1, 1}}};
  datos.valores.assign(2, {});
  for (std::size_t e = 0; e < 20; ++e) {
    datos.valores[0].push_back(static_cast<double>(e % 2));
    datos.valores[1].push_back(static_cast<double>((e / 2) % 2));
    datos.clases.push_back(e % 2);
  }
  return datos;
}

// El mensaje de la excepción que lanza learn, o "" si no lanza std::invalid_argument.
std::string error_de(const inslv::Datos& datos) {
  try {
    inslv::aprender(datos);
  } catch (const std::invalid_argument& error) {
    return error.what();
  }
  return "";
}

TEST(Arreglos, UnValorQueNoSeAdaptaEsUnError) {
  // 0.5 no está en ninguna etiqueta de b (False en 0, True en 1): en el original, exit(1).
  inslv::Datos datos = clase_es_a();
  datos.valores[1][7] = 0.5;
  const std::string mensaje = error_de(datos);
  EXPECT_NE(mensaje.find("del ejemplo 7 en 'b' no se adapta a ninguna de sus etiquetas"),
            std::string::npos)
      << mensaje;
}

TEST(Arreglos, UnaEtiquetaMalFormadaEsUnError) {
  inslv::Datos datos = clase_es_a();
  datos.atributos[0].etiquetas[1] = {"True", 1, 2, 1, 3};
  const std::string mensaje = error_de(datos);
  EXPECT_NE(mensaje.find("la etiqueta 'True' de 'a' no cumple a <= b <= c <= d"), std::string::npos)
      << mensaje;

  datos = clase_es_a();
  datos.clase.etiquetas[0].d = std::numeric_limits<double>::quiet_NaN();
  EXPECT_NE(error_de(datos).find("de 'Clase' no cumple"), std::string::npos);
}

TEST(Arreglos, LosDesconocidosNoSeComprueban) {
  // NaN y el valor que usa la implementación para lo desconocido se adaptan a todo.
  inslv::Datos datos = clase_es_a();
  datos.valores[1][3] = std::numeric_limits<double>::quiet_NaN();
  datos.valores[1][4] = -999999999;
  EXPECT_FALSE(inslv::aprender(datos).empty());
}

TEST(Arreglos, Patron1ReglaSinEsaRegla) {
  // En el original, con R_mix vacío se leía la regla -1 y el programa reventaba.
  inslv::ruleset reglas;
  const inslv::VectorVar variables;
  EXPECT_EQ(inslv::Patron1Regla(reglas, variables, -1), "");
  EXPECT_EQ(inslv::Patron1Regla(reglas, variables, 0), "");
}

TEST(Arreglos, LosErroresInternosSonExcepciones) {
  // En el original, exit(1).
  const inslv::domain_t vacio;
  EXPECT_THROW(static_cast<void>(vacio.Adaptacion(0.5, 0)), std::logic_error);
  EXPECT_THROW(const inslv::example_set conjunto(-1, 3), std::logic_error);
}

TEST(Arreglos, ConjuntosSinEjemplos) {
  // Con 0 ejemplos, los vectores se reservan y ahora también se liberan (asan lo comprueba).
  const inslv::example_set vacio(0, 3);
  inslv::example_set copia;
  copia = vacio;
  const inslv::example_set& misma = copia;
  copia = misma;
  EXPECT_EQ(copia.N_Examples(), 0);
}

}  // namespace
