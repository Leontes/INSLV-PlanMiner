// SPDX-License-Identifier: GPL-3.0-only
// El conjunto de reglas: su semántica de valor, la inferencia y lo que el original hacía con
// memoria sin inicializar. Que las reglas que salen de aprender son las del original, en
// tests/equivalencia y tests/oraculo.

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/codigo.hpp"
#include "inslv/inslv.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace {

inslv::Variable logica(const std::string& nombre) {
  return {nombre, {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}};
}

inslv::difuso::Variables variables() {
  inslv::Datos datos;
  datos.atributos = {logica("a"), logica("b")};
  datos.clase = {"Clase", {{"No", 0, 0, 0, 0}, {"Si", 1, 1, 1, 1}}};
  return inslv::difuso::Variables(datos);
}

// Una regla: bits, las etiquetas que admiten a y b, seguidas; niveles, los de a y b (el umbral es
// 0,5).
inslv::genetico::Codigo regla(const std::string& bits, int clase, double nivel_a, double nivel_b) {
  return {bits, clase, {nivel_a, nivel_b, 0.5}};
}

inslv::reglas::ConjuntoDeReglas conjunto() {
  inslv::reglas::ConjuntoDeReglas reglas;
  reglas.poner_variables(variables());
  return reglas;
}

// El ejemplo (a, b).
inslv::datos::Ejemplo ejemplo(double a, double b) { return inslv::datos::Ejemplo({a, b}); }

TEST(ConjuntoDeReglas, ElPesoSaleDeLosAciertosYLosFallos) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 3, 1, false);
  EXPECT_DOUBLE_EQ(reglas.peso(0), 4.0 / 5.0);
  EXPECT_FALSE(reglas.modificada(0));

  reglas.sumar_ejemplos(0, 1, 2);
  EXPECT_DOUBLE_EQ(reglas.peso(0), 5.0 / 8.0);

  reglas.sumar_aciertos(0, 2);
  EXPECT_DOUBLE_EQ(reglas.peso(0), 5.0 / 8.0);
  reglas.recalcular_peso();
  EXPECT_DOUBLE_EQ(reglas.peso(0), 7.0 / 10.0);
}

TEST(ConjuntoDeReglas, CopiarNoComparteLasReglas) {
  // En el original, la copia implícita compartía los punteros y se liberaban dos veces.
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 0.5);
  inslv::reglas::ConjuntoDeReglas copia(reglas);
  copia.codigo(0).etiquetas[0] = '1';
  copia.anadir(regla("1011", 0, 1, 0), 0.5);

  EXPECT_EQ(reglas.numero_de_reglas(), 1);
  EXPECT_EQ(reglas.codigo(0).etiquetas[0], '0');
  EXPECT_EQ(copia.codigo(0).etiquetas[0], '1');
}

TEST(ConjuntoDeReglas, AsignarSinDominioNoFalla) {
  // En el original, leía el dominio de x aunque no lo tuviera.
  const inslv::reglas::ConjuntoDeReglas vacio;
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas = vacio;
  EXPECT_EQ(reglas.numero_de_reglas(), 0);
}

TEST(ConjuntoDeReglas, AddRulesetAnadeAlPrincipio) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 0.5);
  inslv::reglas::ConjuntoDeReglas otras = conjunto();
  otras.anadir(regla("1011", 0, 1, 0), 0.25);
  otras.anadir(regla("1101", 0, 0, 1), 0);
  otras.marcar_modificada(0, false);

  reglas.anadir_al_principio(otras);

  ASSERT_EQ(reglas.numero_de_reglas(), 3);
  EXPECT_EQ(reglas.clase(0), 0);
  EXPECT_EQ(reglas.peso(0), 0.25);
  EXPECT_EQ(reglas.peso(1), 0);
  EXPECT_EQ(reglas.clase(2), 1);
  EXPECT_EQ(reglas.peso(2), 0.5);
  // Como en el original: modificada queda cierta salvo con peso 0.
  EXPECT_TRUE(reglas.modificada(0));
  EXPECT_FALSE(reglas.modificada(1));
  EXPECT_TRUE(reglas.modificada(2));
}

TEST(ConjuntoDeReglas, QuitarReglas) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 0, 1, 0), 0.25);
  reglas.anadir(regla("1011", 1, 1, 0), 0.5);
  reglas.anadir(regla("1101", 0, 0, 1), 0.75);

  reglas.quitar(5);
  reglas.quitar(-1);
  EXPECT_EQ(reglas.numero_de_reglas(), 3);

  reglas.quitar(0);
  ASSERT_EQ(reglas.numero_de_reglas(), 2);
  EXPECT_EQ(reglas.peso(0), 0.5);

  reglas.quitar_ultima();
  ASSERT_EQ(reglas.numero_de_reglas(), 1);
  EXPECT_EQ(reglas.peso(0), 0.5);
}

TEST(ConjuntoDeReglas, UnaReglaQueNoExisteEsUnError) {
  // En el original, codigo avisaba y devolvía un puntero sin inicializar, y los demás
  // leían otra memoria: Bondad pedía el peso de una regla que no existe.
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 0.5);
  EXPECT_THROW(static_cast<void>(reglas.codigo(1)), std::logic_error);
  EXPECT_THROW(static_cast<void>(reglas.codigo(-1)), std::logic_error);
  EXPECT_THROW(static_cast<void>(reglas.peso(1)), std::logic_error);
  EXPECT_THROW(static_cast<void>(reglas.clase(1)), std::logic_error);
  EXPECT_THROW(reglas.sumar_aciertos(1, 1), std::logic_error);
}

TEST(ConjuntoDeReglas, SinReglasNoSeLeeNinguna) {
  // En el original, leían la regla 0 aunque no hubiera ninguna.
  const inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  std::vector<int> frecuencia(2, 7);
  EXPECT_EQ(reglas.frecuencia_de_atributos(frecuencia), 0);
  EXPECT_EQ(frecuencia, (std::vector<int>{0, 0}));
  EXPECT_EQ(reglas.numero_de_condiciones(), 0);
}

TEST(ConjuntoDeReglas, SoloCuentanLosAtributosRelevantes) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  // a es relevante; b tiene un nivel por debajo del umbral.
  reglas.anadir(regla("0110", 1, 1, 0), 0.5);
  // a admite todas sus etiquetas: no es relevante aunque llegue al umbral; b sí.
  reglas.anadir(regla("1110", 0, 1, 1), 0.5);

  const std::vector<inslv::Condicion> primera = reglas.antecedente(0);
  ASSERT_EQ(primera.size(), 1U);
  EXPECT_EQ(primera[0].atributo, 0U);
  EXPECT_EQ(primera[0].etiquetas, std::vector<std::size_t>{1});

  const std::vector<inslv::Condicion> segunda = reglas.antecedente(1);
  ASSERT_EQ(segunda.size(), 1U);
  EXPECT_EQ(segunda[0].atributo, 1U);
  EXPECT_EQ(segunda[0].etiquetas, std::vector<std::size_t>{0});

  EXPECT_EQ(reglas.numero_de_condiciones(), 2);
  std::vector<int> frecuencia(2);
  EXPECT_EQ(reglas.frecuencia_de_atributos(frecuencia), 1);
  EXPECT_EQ(frecuencia, (std::vector<int>{1, 1}));

  EXPECT_EQ(reglas.atributos_sobre_el_umbral(0), 1);
  EXPECT_EQ(reglas.atributos_sobre_el_umbral(1), 2);
  EXPECT_EQ(reglas.etiquetas_excluidas(0), 1);
  EXPECT_EQ(reglas.etiquetas_excluidas(1), 1);
  EXPECT_DOUBLE_EQ(reglas.atributos_por_regla(), 1.5);
}

TEST(ConjuntoDeReglas, ElTextoDeUnaRegla) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 3, 1, false);
  EXPECT_EQ(reglas.texto(0), "IF\n\ta = { True}\nTHEN Clase IS Si   W 0.8 and N+= 3 and N-= 1\n");
  EXPECT_EQ(reglas.texto_del_consecuente(0), "Si");
}

TEST(ConjuntoDeReglas, InfiereLaClaseDeLaMejorRegla) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("1011", 0, 1, 0), 0.25);  // Si a es False, No.
  reglas.anadir(regla("0111", 1, 1, 0), 0.5);   // Si a es True, Si.
  reglas.anadir(regla("0111", 0, 1, 0), 0.75);  // Si a es True, No: gana por peso.

  double grado = -1;
  int disparada = -2;
  EXPECT_EQ(reglas.inferir(ejemplo(1, 0), grado, disparada), 0);
  EXPECT_EQ(disparada, 2);
  EXPECT_EQ(grado, 0.75);

  EXPECT_EQ(reglas.inferir(ejemplo(0, 0), grado, disparada), 0);
  EXPECT_EQ(disparada, 0);
  EXPECT_EQ(grado, 0.25);
}

TEST(ConjuntoDeReglas, SinReglaQueSeAdapteNoHayClase) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 0.5);

  double grado = -1;
  int disparada = -2;
  EXPECT_EQ(reglas.inferir(ejemplo(0, 0), grado, disparada), -1);
  EXPECT_EQ(disparada, -1);
  EXPECT_EQ(grado, -1);  // No lo cambia.

  double grado_p = 1;
  double peso_p = 1;
  double grado_n = 1;
  double peso_n = 1;
  int aislada = 0;
  EXPECT_EQ(reglas.inferir(ejemplo(0, 0), 1, grado_p, peso_p, grado_n, peso_n, disparada, aislada),
            -1);
  EXPECT_EQ(grado_p, 0);
  EXPECT_EQ(grado_n, 0);
  EXPECT_EQ(aislada, -1000);
}

TEST(ConjuntoDeReglas, CuentaLasReglasQueGananALasDeOtraClase) {
  inslv::reglas::ConjuntoDeReglas reglas = conjunto();
  reglas.anadir(regla("0111", 1, 1, 0), 0.75);  // De su clase: gana.
  reglas.anadir(regla("0111", 1, 1, 0), 0.5);   // De su clase: gana.
  reglas.anadir(regla("0111", 0, 1, 0), 0.25);  // De otra.

  double grado_p = 0;
  double peso_p = 0;
  double grado_n = 0;
  double peso_n = 0;
  int disparada = -1;
  int aislada = 0;
  EXPECT_EQ(reglas.inferir(ejemplo(1, 0), 1, grado_p, peso_p, grado_n, peso_n, disparada, aislada),
            1);
  EXPECT_EQ(disparada, 0);
  EXPECT_EQ(grado_p, 0.75);
  EXPECT_EQ(grado_n, 0.25);
  EXPECT_EQ(aislada, 2);

  // Para un ejemplo de la otra clase, la clasificación falla: gana una de otra (la de 0,75) y
  // también la de 0,5.
  EXPECT_EQ(reglas.inferir(ejemplo(1, 0), 0, grado_p, peso_p, grado_n, peso_n, disparada, aislada),
            1);
  EXPECT_EQ(grado_p, 0.25);
  EXPECT_EQ(grado_n, 0.75);
  EXPECT_EQ(aislada, -2);
}

}  // namespace
