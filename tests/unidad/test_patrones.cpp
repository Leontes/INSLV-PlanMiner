// SPDX-License-Identifier: GPL-3.0-only
// Los patrones de las reglas y cómo se combinan las compatibles.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "inslv/aprendizaje/patrones.hpp"
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

// Una regla con los dos atributos por encima del umbral.
inslv::genetico::Codigo regla(const std::string& etiquetas, int clase) {
  return {etiquetas, clase, {1, 1, 0.5}};
}

inslv::reglas::ConjuntoDeReglas conjunto(const inslv::difuso::Variables& V) {
  inslv::reglas::ConjuntoDeReglas reglas;
  reglas.poner_variables(V);
  return reglas;
}

TEST(Patrones, ElPatronDeUnaRegla) {
  const inslv::difuso::Variables V = variables();
  inslv::reglas::ConjuntoDeReglas R = conjunto(V);
  R.anadir(regla("0101", 1), 0.5);
  EXPECT_EQ(inslv::aprendizaje::patron_de_regla(R, 0), "0101");
  EXPECT_EQ(inslv::aprendizaje::patron_de_regla(R, 1), "");
}

TEST(Patrones, ElPatronDeDosReglasEmparejadas) {
  const inslv::difuso::Variables V = variables();
  inslv::reglas::ConjuntoDeReglas descriptiva = conjunto(V);
  inslv::reglas::ConjuntoDeReglas discriminante = conjunto(V);
  descriptiva.anadir(regla("0101", 1), 0.5);
  discriminante.anadir(regla("0111", 1), 0.5);
  EXPECT_EQ(inslv::aprendizaje::patron_de_dos_reglas(descriptiva, discriminante, V, 0), "01?1");
}

TEST(Patrones, UnAtributoPorDebajoDelUmbralAdmiteTodas) {
  const inslv::difuso::Variables V = variables();
  inslv::reglas::ConjuntoDeReglas descriptiva = conjunto(V);
  inslv::reglas::ConjuntoDeReglas discriminante = conjunto(V);
  descriptiva.anadir(regla("0101", 1), 0.5);
  // b está por debajo del umbral: sus etiquetas valen como si admitiera las dos.
  discriminante.anadir({"0100", 1, {1, 0, 0.5}}, 0.5);
  EXPECT_EQ(inslv::aprendizaje::patron_de_dos_reglas(descriptiva, discriminante, V, 0), "01?1");
}

TEST(Patrones, EspecificacionDeOtra) {
  EXPECT_TRUE(inslv::aprendizaje::es_especificacion("0100", "0111"));
  EXPECT_TRUE(inslv::aprendizaje::es_especificacion("0111", "0111"));
  EXPECT_FALSE(inslv::aprendizaje::es_especificacion("1000", "0111"));
}

TEST(Patrones, FiltrarQuitaLasReglasDeLosTresConjuntos) {
  const inslv::difuso::Variables V = variables();
  inslv::reglas::ConjuntoDeReglas R = conjunto(V);
  inslv::reglas::ConjuntoDeReglas R_dis = conjunto(V);
  inslv::reglas::ConjuntoDeReglas R_mix = conjunto(V);
  for (const double peso : {0.25, 0.5, 0.75}) {
    R.anadir(regla("0101", 1), peso);
    R_dis.anadir(regla("0101", 1), peso);
    R_mix.anadir(regla("0101", 1), peso);
  }

  inslv::aprendizaje::quitar_reglas(R, R_dis, R_mix, {0, 2});

  for (const inslv::reglas::ConjuntoDeReglas* reglas : {&R, &R_dis, &R_mix}) {
    ASSERT_EQ(reglas->numero_de_reglas(), 1);
    EXPECT_EQ(reglas->peso(0), 0.5);
  }
}

class Analizar : public testing::Test {
 protected:
  // Añade una regla a los tres conjuntos, con su patrón.
  void anade(const std::string& descriptiva, const std::string& discriminante,
             const std::string& patron, int clase, double mas, double menos) {
    R_Descrip.anadir(regla(descriptiva, clase), mas, menos);
    R.anadir(regla(discriminante, clase), mas, menos);
    R_mix.anadir(regla(patron, clase), mas, menos);
  }

  inslv::difuso::Variables V = variables();
  inslv::reglas::ConjuntoDeReglas R = conjunto(V);
  inslv::reglas::ConjuntoDeReglas R_Descrip = conjunto(V);
  inslv::reglas::ConjuntoDeReglas R_mix = conjunto(V);
};

TEST_F(Analizar, CombinaDosReglasCompatibles) {
  anade("0101", "0111", "01?1", 1, 3, 1);
  anade("0110", "0111", "011?", 1, 5, 2);

  inslv::aprendizaje::combinar_reglas(V, R, R_Descrip, R_mix, 1);

  // La nueva (la primera) se va y la otra se queda con la combinación y sus ejemplos.
  ASSERT_EQ(R.numero_de_reglas(), 1);
  EXPECT_EQ(R.codigo(0).etiquetas, "0111");
  EXPECT_EQ(R_Descrip.codigo(0).etiquetas, "0111");
  EXPECT_EQ(R_mix.codigo(0).etiquetas, "0111");
  EXPECT_EQ(R.aciertos(0), 8);
  EXPECT_EQ(R.fallos(0), 3);
}

TEST_F(Analizar, QuitaUnaReglaConElMismoPatronQueOtraDeSuClase) {
  anade("0101", "0111", "01?1", 1, 3, 1);
  anade("0101", "0111", "01?1", 1, 5, 2);

  inslv::aprendizaje::combinar_reglas(V, R, R_Descrip, R_mix, 1);

  ASSERT_EQ(R.numero_de_reglas(), 1);
  EXPECT_EQ(R.aciertos(0), 5);
}

TEST_F(Analizar, ContradiccionConElMismoPatronDeOtraClase) {
  anade("0101", "0111", "01?1", 1, 3, 1);
  anade("0101", "0111", "01?1", 0, 5, 2);

  inslv::aprendizaje::combinar_reglas(V, R, R_Descrip, R_mix, 1);

  // No se quita ninguna, pero a la nueva sus aciertos le cuentan como fallos.
  ASSERT_EQ(R.numero_de_reglas(), 2);
  EXPECT_EQ(R.fallos(0), 4);
  EXPECT_DOUBLE_EQ(R.peso(0), 4.0 / 8.0);
  EXPECT_DOUBLE_EQ(R_Descrip.peso(0), 4.0 / 8.0);
}

TEST_F(Analizar, NoTocaLasIncompatibles) {
  anade("0101", "0101", "0101", 1, 3, 1);
  anade("1001", "1001", "1001", 1, 5, 2);

  inslv::aprendizaje::combinar_reglas(V, R, R_Descrip, R_mix, 1);

  ASSERT_EQ(R.numero_de_reglas(), 2);
  EXPECT_EQ(R.codigo(0).etiquetas, "0101");
  EXPECT_EQ(R.codigo(1).etiquetas, "1001");
}

}  // namespace
