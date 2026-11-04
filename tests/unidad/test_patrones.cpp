// SPDX-License-Identifier: GPL-3.0-only
// Los patrones de las reglas y cómo se combinan las compatibles.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "inslv/aprendizaje/patrones.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/genetico/genetcode.h"
#include "inslv/inslv.hpp"
#include "inslv/reglas/ruleset.h"

namespace {

inslv::Variable logica(const std::string& nombre) {
  return {nombre, {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}};
}

inslv::VectorVar variables() {
  inslv::Datos datos;
  datos.atributos = {logica("a"), logica("b")};
  datos.clase = {"Clase", {{"No", 0, 0, 0, 0}, {"Si", 1, 1, 1, 1}}};
  return inslv::VectorVar(datos);
}

// Una regla con los dos atributos por encima del umbral.
inslv::genetcode regla(const std::string& etiquetas, int clase) {
  return {etiquetas, clase, {1, 1, 0.5}};
}

inslv::ruleset conjunto(const inslv::VectorVar& V) {
  inslv::ruleset reglas;
  reglas.AddDomain(V);
  return reglas;
}

TEST(Patrones, ElPatronDeUnaRegla) {
  const inslv::VectorVar V = variables();
  inslv::ruleset R = conjunto(V);
  R.Add(regla("0101", 1), 0.5);
  EXPECT_EQ(inslv::Patron1Regla(R, 0), "0101");
  EXPECT_EQ(inslv::Patron1Regla(R, 1), "");
}

TEST(Patrones, ElPatronDeDosReglasEmparejadas) {
  const inslv::VectorVar V = variables();
  inslv::ruleset descriptiva = conjunto(V);
  inslv::ruleset discriminante = conjunto(V);
  descriptiva.Add(regla("0101", 1), 0.5);
  discriminante.Add(regla("0111", 1), 0.5);
  EXPECT_EQ(inslv::Patron2Reglas(descriptiva, discriminante, V, 0), "01?1");
}

TEST(Patrones, UnAtributoPorDebajoDelUmbralAdmiteTodas) {
  const inslv::VectorVar V = variables();
  inslv::ruleset descriptiva = conjunto(V);
  inslv::ruleset discriminante = conjunto(V);
  descriptiva.Add(regla("0101", 1), 0.5);
  // b está por debajo del umbral: sus etiquetas valen como si admitiera las dos.
  discriminante.Add({"0100", 1, {1, 0, 0.5}}, 0.5);
  EXPECT_EQ(inslv::Patron2Reglas(descriptiva, discriminante, V, 0), "01?1");
}

TEST(Patrones, EspecificacionDeOtra) {
  EXPECT_TRUE(inslv::ComprobacionEspGen("0100", "0111"));
  EXPECT_TRUE(inslv::ComprobacionEspGen("0111", "0111"));
  EXPECT_FALSE(inslv::ComprobacionEspGen("1000", "0111"));
}

TEST(Patrones, FiltrarQuitaLasReglasDeLosTresConjuntos) {
  const inslv::VectorVar V = variables();
  inslv::ruleset R = conjunto(V);
  inslv::ruleset R_dis = conjunto(V);
  inslv::ruleset R_mix = conjunto(V);
  for (const double peso : {0.25, 0.5, 0.75}) {
    R.Add(regla("0101", 1), peso);
    R_dis.Add(regla("0101", 1), peso);
    R_mix.Add(regla("0101", 1), peso);
  }

  inslv::Filtrar_Reglas_New(R, R_dis, R_mix, {0, 2});

  for (const inslv::ruleset* reglas : {&R, &R_dis, &R_mix}) {
    ASSERT_EQ(reglas->N_rule(), 1);
    EXPECT_EQ(reglas->Get_Weight(0), 0.5);
  }
}

class Analizar : public testing::Test {
 protected:
  // Añade una regla a los tres conjuntos, con su patrón.
  void anade(const std::string& descriptiva, const std::string& discriminante,
             const std::string& patron, int clase, double mas, double menos) {
    R_Descrip.Add(regla(descriptiva, clase), mas, menos, false);
    R.Add(regla(discriminante, clase), mas, menos, false);
    R_mix.Add(regla(patron, clase), mas, menos, false);
  }

  inslv::VectorVar V = variables();
  inslv::ruleset R = conjunto(V);
  inslv::ruleset R_Descrip = conjunto(V);
  inslv::ruleset R_mix = conjunto(V);
};

TEST_F(Analizar, CombinaDosReglasCompatibles) {
  anade("0101", "0111", "01?1", 1, 3, 1);
  anade("0110", "0111", "011?", 1, 5, 2);

  inslv::AnalizarConjuntosDeReglas(V, R, R_Descrip, R_mix, 1);

  // La nueva (la primera) se va y la otra se queda con la combinación y sus ejemplos.
  ASSERT_EQ(R.N_rule(), 1);
  EXPECT_EQ(R.Get_Code_Rule(0).etiquetas, "0111");
  EXPECT_EQ(R_Descrip.Get_Code_Rule(0).etiquetas, "0111");
  EXPECT_EQ(R_mix.Get_Code_Rule(0).etiquetas, "0111");
  EXPECT_EQ(R.Get_N_Mas(0), 8);
  EXPECT_EQ(R.Get_N_Menos(0), 3);
  EXPECT_TRUE(R.Get_Modificada(0));
}

TEST_F(Analizar, QuitaUnaReglaConElMismoPatronQueOtraDeSuClase) {
  anade("0101", "0111", "01?1", 1, 3, 1);
  anade("0101", "0111", "01?1", 1, 5, 2);

  inslv::AnalizarConjuntosDeReglas(V, R, R_Descrip, R_mix, 1);

  ASSERT_EQ(R.N_rule(), 1);
  EXPECT_EQ(R.Get_N_Mas(0), 5);
}

TEST_F(Analizar, ContradiccionConElMismoPatronDeOtraClase) {
  anade("0101", "0111", "01?1", 1, 3, 1);
  anade("0101", "0111", "01?1", 0, 5, 2);

  inslv::AnalizarConjuntosDeReglas(V, R, R_Descrip, R_mix, 1);

  // No se quita ninguna, pero a la nueva sus aciertos le cuentan como fallos.
  ASSERT_EQ(R.N_rule(), 2);
  EXPECT_EQ(R.Get_N_Menos(0), 4);
  EXPECT_DOUBLE_EQ(R.Get_Weight(0), 4.0 / 8.0);
  EXPECT_DOUBLE_EQ(R_Descrip.Get_Weight(0), 4.0 / 8.0);
}

TEST_F(Analizar, NoTocaLasIncompatibles) {
  anade("0101", "0101", "0101", 1, 3, 1);
  anade("1001", "1001", "1001", 1, 5, 2);

  inslv::AnalizarConjuntosDeReglas(V, R, R_Descrip, R_mix, 1);

  ASSERT_EQ(R.N_rule(), 2);
  EXPECT_EQ(R.Get_Code_Rule(0).etiquetas, "0101");
  EXPECT_EQ(R.Get_Code_Rule(1).etiquetas, "1001");
}

}  // namespace
