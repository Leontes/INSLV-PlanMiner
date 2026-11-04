// SPDX-License-Identifier: GPL-3.0-only
// La multipoblación, la bondad de un individuo y la búsqueda local que afina su regla.

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

#include "inslv/aprendizaje/aprender.h"
#include "inslv/aprendizaje/bondad.h"
#include "inslv/aprendizaje/tabla.h"
#include "inslv/datos/example_set.h"
#include "inslv/difuso/VectorVar.h"
#include "inslv/genetico/genetcode.h"
#include "inslv/genetico/multipoblacion.h"
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

// 8 ejemplos: la clase es el valor de a; b no tiene relación con ella.
inslv::example_set ejemplos() {
  return inslv::example_set(
      {{0, 1, 0, 1, 0, 1, 0, 1}, {0, 0, 1, 1, 0, 0, 1, 1}, {0, 1, 0, 1, 0, 1, 0, 1}});
}

inslv::multipoblacion poblacion() { return {2, 20, 4, 3, 0.5, 0, 0.01}; }

class Bondad : public testing::Test {
 protected:
  // La bondad de la regla con estos valores en la posición 0 de la población.
  double valora(const inslv::genetcode& code, inslv::Fitness& valor) {
    G.PutCodigo(0, code);
    const inslv::Evaluacion ev = evaluacion();
    return inslv::Bondad(ev, 0, valor, eliminables);
  }

  [[nodiscard]] inslv::Evaluacion evaluacion() {
    return {G, V, E, R, false, por_clase, tabla, basica, errores, 0};
  }

  inslv::VectorVar V = variables();
  inslv::example_set E = ejemplos();
  inslv::TablaDeAdaptaciones tabla{V, E};
  inslv::multipoblacion G = poblacion();
  inslv::ruleset R;
  std::vector<int> por_clase = E.Examples_per_Class(2, 2);
  std::vector<int> basica = std::vector<int>(1);
  std::vector<int> errores = std::vector<int>(1);
  std::vector<int> eliminables = {7};
};

TEST(Multipoblacion, ElCodigoEsElDelIndividuo) {
  inslv::multipoblacion G = poblacion();
  const inslv::genetcode code{"0110", 1, {0.1, 0.2, 0.3}};
  G.PutCodigo(5, code);
  EXPECT_EQ(G.Etiquetas(5), "0110");
  EXPECT_EQ(G.ClaseIndividuo(5), 1);
  EXPECT_EQ(G.Niveles(5), (std::vector<double>{0.1, 0.2, 0.3}));
  const inslv::genetcode copia = G.Codigo(5);
  EXPECT_EQ(copia.etiquetas, code.etiquetas);
  EXPECT_EQ(copia.clase, code.clase);
  EXPECT_EQ(copia.niveles, code.niveles);
}

TEST(Multipoblacion, OrdenaPorValoracionSinCambiarElOrdenDeLosIguales) {
  inslv::multipoblacion G(2, 4, 4, 3, 0.5, 0, 0.01);
  // Cada individuo se distingue por su clase y su primer bit.
  const std::vector<inslv::genetcode> codigos = {{"0000", 0, {0, 0, 0}},
                                                 {"1000", 0, {0, 0, 0}},
                                                 {"0000", 1, {0, 0, 0}},
                                                 {"1000", 1, {0, 0, 0}}};
  const std::vector<double> valor = {1, 3, 1, 2};
  for (std::size_t i = 0; i < codigos.size(); ++i) {
    G.PutCodigo(static_cast<int>(i), codigos[i]);
    G.Valorar(static_cast<int>(i), {valor[i]});
  }

  G.Sort();

  EXPECT_EQ(G.Valoracion(0)[0], 3);
  EXPECT_EQ(G.Etiquetas(0), "1000");
  EXPECT_EQ(G.ClaseIndividuo(0), 0);
  EXPECT_EQ(G.Valoracion(1)[0], 2);
  EXPECT_EQ(G.Valoracion(2)[0], 1);
  EXPECT_EQ(G.ClaseIndividuo(2), 0);
  EXPECT_EQ(G.Valoracion(3)[0], 1);
  EXPECT_EQ(G.ClaseIndividuo(3), 1);
}

TEST(Multipoblacion, ValorarQuitaLaMarcaDeModificado) {
  inslv::multipoblacion G = poblacion();
  EXPECT_TRUE(G.Modificado(0));
  G.Valorar(0, {1});
  EXPECT_FALSE(G.Modificado(0));
  G.PutModificado(0);
  EXPECT_TRUE(G.Modificado(0));
}

TEST_F(Bondad, ElPesoDeUnIndividuo) {
  double n_mas = 0;
  double n_menos = 0;
  // Si a es True, Si: acierta los cuatro ejemplos que cubre.
  G.PutCodigo(0, {"0111", 1, {1, 0, 0.5}});
  EXPECT_EQ(inslv::Weight_Individual(G, E, tabla, 0, n_mas, n_menos), 1);
  EXPECT_EQ(n_mas, 4);
  EXPECT_EQ(n_menos, 0);
  // Si a es True, No: los falla.
  G.PutCodigo(0, {"0111", 0, {1, 0, 0.5}});
  EXPECT_DOUBLE_EQ(inslv::Weight_Individual(G, E, tabla, 0, n_mas, n_menos), 1.0 / 5.0);
  EXPECT_EQ(n_mas, 0);
  EXPECT_EQ(n_menos, 4);
}

TEST_F(Bondad, UnaReglaSinAtributosNoEsValida) {
  inslv::Fitness valor{};
  EXPECT_EQ(valora({"1111", 1, {0, 0, 0.5}}, valor), 0);
  for (const double v : valor) {
    EXPECT_EQ(v, -16);
  }
  // Si la regla no es válida, las reglas a eliminar no cambian.
  EXPECT_EQ(eliminables, std::vector<int>{7});
}

TEST_F(Bondad, UnaReglaQueNoAdmiteNingunaEtiquetaNoEsValida) {
  inslv::Fitness valor{};
  EXPECT_EQ(valora({"0011", 1, {1, 0, 0.5}}, valor), 0);
  EXPECT_EQ(valor[0], -16);
}

TEST_F(Bondad, UnAtributoQueAdmiteTodasSusEtiquetasNoEsRelevante) {
  inslv::Fitness valor{};
  valora({"0111", 1, {1, 1, 0.5}}, valor);
  // b admite todas: le baja el nivel por debajo del umbral.
  EXPECT_LT(G.Niveles(0)[1], 0.5);
  EXPECT_EQ(G.Niveles(0)[0], 1);
}

TEST_F(Bondad, UnaReglaQueSoloAciertaEsMejorQueUnaQueTambienFalla) {
  inslv::Fitness acierta{};
  inslv::Fitness falla{};
  EXPECT_EQ(valora({"0111", 1, {1, 0, 0.5}}, acierta), 1);
  EXPECT_DOUBLE_EQ(valora({"0111", 0, {1, 0, 0.5}}, falla), 1.0 / 5.0);
  EXPECT_GT(acierta[0], falla[0]);
  // Sin más reglas, nada que eliminar.
  EXPECT_TRUE(eliminables.empty());
}

TEST_F(Bondad, GeneralizarQuitaElAtributoQueNoHaceFalta) {
  // Si a es True y b es True, Si: sin b acierta el doble.
  G.PutCodigo(0, {"0101", 1, {1, 1, 0.5}});
  const inslv::Evaluacion ev = evaluacion();
  inslv::Obtener_Regla_Discriminante(ev, 0, eliminables);
  EXPECT_EQ(G.Niveles(0)[0], 1);
  EXPECT_LT(G.Niveles(0)[1], 0.5);
  EXPECT_FALSE(G.Modificado(0));
}

TEST_F(Bondad, EspecializarQuitaLasEtiquetasQueNoHacenFalta) {
  // Si a es False o True, Si: acierta la mitad y falla la otra; con a en True solo acierta.
  G.PutCodigo(0, {"1111", 1, {1, 0, 0.5}});
  const inslv::Evaluacion ev = evaluacion();
  inslv::Obtener_Regla_Descriptiva(ev, 0, eliminables);
  EXPECT_EQ(G.Etiquetas(0).substr(0, 2), "01");
  EXPECT_FALSE(G.Modificado(0));
}

TEST(Aciertos, AnotaLoQueDaCadaEjemplo) {
  const inslv::VectorVar V = variables();
  inslv::example_set E = ejemplos();
  inslv::ruleset R;
  R.AddDomain(V);
  // Si a es True, Si: acierta los cuatro con a a 1 y no dice nada de los otros.
  R.Add({"0111", 1, {1, 0, 0.5}}, 0.5);

  EXPECT_EQ(inslv::Aciertos(R, V, E), 0.5);

  EXPECT_EQ(E.Not_Covered_Examples(), 4);
  EXPECT_FALSE(E.Is_Covered(0));
  EXPECT_EQ(E.Rule_Used(0), -1);
  EXPECT_TRUE(E.Is_Covered(1));
  EXPECT_EQ(E.Rule_Used(1), 0);
  EXPECT_EQ(E.Grade_Is_Positive_Covered(1), 0.5);
}

TEST(Aciertos, LasReglasQueDisparan) {
  const inslv::VectorVar V = variables();
  const inslv::example_set E = ejemplos();
  inslv::ruleset R;
  R.AddDomain(V);
  R.Add({"0111", 1, {1, 0, 0.5}}, 0.5);   // Cubre la mitad.
  R.Add({"0111", 0, {1, 0, 0.5}}, 0.25);  // Cubre lo mismo, con menos peso: nunca gana.
  R.Add({"1011", 0, {1, 0, 0.5}}, 0.5);   // Cubre la otra mitad.
  EXPECT_EQ(inslv::Reglas_Que_Disparan(R, E), (std::vector<int>{0, 2}));
}

}  // namespace
