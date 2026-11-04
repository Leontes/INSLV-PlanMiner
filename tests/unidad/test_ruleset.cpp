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
#include "inslv/genetico/genetcode.h"
#include "inslv/inslv.hpp"
#include "inslv/reglas/ruleset.h"

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
inslv::genetcode regla(const std::string& bits, int clase, double nivel_a, double nivel_b) {
  return {bits, clase, {nivel_a, nivel_b, 0.5}};
}

inslv::ruleset conjunto() {
  inslv::ruleset reglas;
  reglas.AddDomain(variables());
  return reglas;
}

// El ejemplo (a, b).
inslv::datos::Ejemplo ejemplo(double a, double b) { return inslv::datos::Ejemplo({a, b}); }

TEST(Ruleset, ElPesoSaleDeLosAciertosYLosFallos) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 3, 1, false);
  EXPECT_DOUBLE_EQ(reglas.Get_Weight(0), 4.0 / 5.0);
  EXPECT_FALSE(reglas.Get_Modificada(0));

  reglas.Add_Weight(0, 1, 2);
  EXPECT_DOUBLE_EQ(reglas.Get_Weight(0), 5.0 / 8.0);

  reglas.Add_N_Mas(0, 2);
  EXPECT_DOUBLE_EQ(reglas.Get_Weight(0), 5.0 / 8.0);
  reglas.Recalculate_Weight();
  EXPECT_DOUBLE_EQ(reglas.Get_Weight(0), 7.0 / 10.0);
}

TEST(Ruleset, CopiarNoComparteLasReglas) {
  // En el original, la copia implícita compartía los punteros y se liberaban dos veces.
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 0.5);
  inslv::ruleset copia(reglas);
  copia.Get_Code_Rule(0).etiquetas[0] = '1';
  copia.Add(regla("1011", 0, 1, 0), 0.5);

  EXPECT_EQ(reglas.N_rule(), 1);
  EXPECT_EQ(reglas.Get_Code_Rule(0).etiquetas[0], '0');
  EXPECT_EQ(copia.Get_Code_Rule(0).etiquetas[0], '1');
}

TEST(Ruleset, AsignarSinDominioNoFalla) {
  // En el original, leía el dominio de x aunque no lo tuviera.
  const inslv::ruleset vacio;
  inslv::ruleset reglas = conjunto();
  reglas = vacio;
  EXPECT_EQ(reglas.N_rule(), 0);
}

TEST(Ruleset, AddRulesetAnadeAlPrincipio) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 0.5);
  inslv::ruleset otras = conjunto();
  otras.Add(regla("1011", 0, 1, 0), 0.25);
  otras.Add(regla("1101", 0, 0, 1), 0);
  otras.Modificada(0, false);

  reglas.AddRuleset(otras);

  ASSERT_EQ(reglas.N_rule(), 3);
  EXPECT_EQ(reglas.Class_of_rule(0), 0);
  EXPECT_EQ(reglas.Get_Weight(0), 0.25);
  EXPECT_EQ(reglas.Get_Weight(1), 0);
  EXPECT_EQ(reglas.Class_of_rule(2), 1);
  EXPECT_EQ(reglas.Get_Weight(2), 0.5);
  // Como en el original: modificada queda cierta salvo con peso 0.
  EXPECT_TRUE(reglas.Get_Modificada(0));
  EXPECT_FALSE(reglas.Get_Modificada(1));
  EXPECT_TRUE(reglas.Get_Modificada(2));
}

TEST(Ruleset, QuitarReglas) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 0, 1, 0), 0.25);
  reglas.Add(regla("1011", 1, 1, 0), 0.5);
  reglas.Add(regla("1101", 0, 0, 1), 0.75);

  reglas.Remove(5);
  reglas.Remove(-1);
  EXPECT_EQ(reglas.N_rule(), 3);

  reglas.Remove(0);
  ASSERT_EQ(reglas.N_rule(), 2);
  EXPECT_EQ(reglas.Get_Weight(0), 0.5);

  reglas.Remove();
  ASSERT_EQ(reglas.N_rule(), 1);
  EXPECT_EQ(reglas.Get_Weight(0), 0.5);
}

TEST(Ruleset, UnaReglaQueNoExisteEsUnError) {
  // En el original, Get_Code_Rule avisaba y devolvía un puntero sin inicializar, y los demás
  // leían otra memoria: Bondad pedía el peso de una regla que no existe.
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 0.5);
  EXPECT_THROW(static_cast<void>(reglas.Get_Code_Rule(1)), std::logic_error);
  EXPECT_THROW(static_cast<void>(reglas.Get_Code_Rule(-1)), std::logic_error);
  EXPECT_THROW(static_cast<void>(reglas.Get_Weight(1)), std::logic_error);
  EXPECT_THROW(static_cast<void>(reglas.Class_of_rule(1)), std::logic_error);
  EXPECT_THROW(reglas.Add_N_Mas(1, 1), std::logic_error);
}

TEST(Ruleset, SinReglasNoSeLeeNinguna) {
  // En el original, leían la regla 0 aunque no hubiera ninguna.
  const inslv::ruleset reglas = conjunto();
  std::vector<int> frecuencia(2, 7);
  EXPECT_EQ(reglas.Frecuence_each_Variables(frecuencia), 0);
  EXPECT_EQ(frecuencia, (std::vector<int>{0, 0}));
  EXPECT_EQ(reglas.Condiciones_per_RB(), 0);
}

TEST(Ruleset, SoloCuentanLosAtributosRelevantes) {
  inslv::ruleset reglas = conjunto();
  // a es relevante; b tiene un nivel por debajo del umbral.
  reglas.Add(regla("0110", 1, 1, 0), 0.5);
  // a admite todas sus etiquetas: no es relevante aunque llegue al umbral; b sí.
  reglas.Add(regla("1110", 0, 1, 1), 0.5);

  const std::vector<inslv::Condicion> primera = reglas.Antecedente(0);
  ASSERT_EQ(primera.size(), 1U);
  EXPECT_EQ(primera[0].atributo, 0U);
  EXPECT_EQ(primera[0].etiquetas, std::vector<std::size_t>{1});

  const std::vector<inslv::Condicion> segunda = reglas.Antecedente(1);
  ASSERT_EQ(segunda.size(), 1U);
  EXPECT_EQ(segunda[0].atributo, 1U);
  EXPECT_EQ(segunda[0].etiquetas, std::vector<std::size_t>{0});

  EXPECT_EQ(reglas.Condiciones_per_RB(), 2);
  std::vector<int> frecuencia(2);
  EXPECT_EQ(reglas.Frecuence_each_Variables(frecuencia), 1);
  EXPECT_EQ(frecuencia, (std::vector<int>{1, 1}));

  EXPECT_EQ(reglas.Variables_of_rule(0), 1);
  EXPECT_EQ(reglas.Variables_of_rule(1), 2);
  EXPECT_EQ(reglas.Numero_Ceros_of_rule(0), 1);
  EXPECT_EQ(reglas.Numero_Ceros_of_rule(1), 1);
  EXPECT_DOUBLE_EQ(reglas.Variables_per_rule(), 1.5);
}

TEST(Ruleset, ElTextoDeUnaRegla) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 3, 1, false);
  EXPECT_EQ(reglas.SPrint(0), "IF\n\ta = { True}\nTHEN Clase IS Si   W 0.8 and N+= 3 and N-= 1\n");
  EXPECT_EQ(reglas.SPrint_Consequent(0), "Si");
}

TEST(Ruleset, InfiereLaClaseDeLaMejorRegla) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("1011", 0, 1, 0), 0.25);  // Si a es False, No.
  reglas.Add(regla("0111", 1, 1, 0), 0.5);   // Si a es True, Si.
  reglas.Add(regla("0111", 0, 1, 0), 0.75);  // Si a es True, No: gana por peso.

  double grado = -1;
  int disparada = -2;
  EXPECT_EQ(reglas.InferenceC(ejemplo(1, 0), grado, disparada), 0);
  EXPECT_EQ(disparada, 2);
  EXPECT_EQ(grado, 0.75);

  EXPECT_EQ(reglas.InferenceC(ejemplo(0, 0), grado, disparada), 0);
  EXPECT_EQ(disparada, 0);
  EXPECT_EQ(grado, 0.25);
}

TEST(Ruleset, SinReglaQueSeAdapteNoHayClase) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 0.5);

  double grado = -1;
  int disparada = -2;
  EXPECT_EQ(reglas.InferenceC(ejemplo(0, 0), grado, disparada), -1);
  EXPECT_EQ(disparada, -1);
  EXPECT_EQ(grado, -1);  // No lo cambia.

  double grado_p = 1;
  double peso_p = 1;
  double grado_n = 1;
  double peso_n = 1;
  int aislada = 0;
  EXPECT_EQ(
      reglas.InferenceC(ejemplo(0, 0), 1, grado_p, peso_p, grado_n, peso_n, disparada, aislada),
      -1);
  EXPECT_EQ(grado_p, 0);
  EXPECT_EQ(grado_n, 0);
  EXPECT_EQ(aislada, -1000);
}

TEST(Ruleset, CuentaLasReglasQueGananALasDeOtraClase) {
  inslv::ruleset reglas = conjunto();
  reglas.Add(regla("0111", 1, 1, 0), 0.75);  // De su clase: gana.
  reglas.Add(regla("0111", 1, 1, 0), 0.5);   // De su clase: gana.
  reglas.Add(regla("0111", 0, 1, 0), 0.25);  // De otra.

  double grado_p = 0;
  double peso_p = 0;
  double grado_n = 0;
  double peso_n = 0;
  int disparada = -1;
  int aislada = 0;
  EXPECT_EQ(
      reglas.InferenceC(ejemplo(1, 0), 1, grado_p, peso_p, grado_n, peso_n, disparada, aislada), 1);
  EXPECT_EQ(disparada, 0);
  EXPECT_EQ(grado_p, 0.75);
  EXPECT_EQ(grado_n, 0.25);
  EXPECT_EQ(aislada, 2);

  // Para un ejemplo de la otra clase, la clasificación falla: gana una de otra (la de 0,75) y
  // también la de 0,5.
  EXPECT_EQ(
      reglas.InferenceC(ejemplo(1, 0), 0, grado_p, peso_p, grado_n, peso_n, disparada, aislada), 1);
  EXPECT_EQ(grado_p, 0.25);
  EXPECT_EQ(grado_n, 0.75);
  EXPECT_EQ(aislada, -2);
}

}  // namespace
