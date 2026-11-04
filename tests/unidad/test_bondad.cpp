// SPDX-License-Identifier: GPL-3.0-only
// La multipoblación, la bondad de un individuo y la búsqueda local que afina su regla.

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

#include "inslv/aprendizaje/aprender.hpp"
#include "inslv/aprendizaje/bondad.hpp"
#include "inslv/aprendizaje/tabla.hpp"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/codigo.hpp"
#include "inslv/genetico/multipoblacion.hpp"
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

// 8 ejemplos: la clase es el valor de a; b no tiene relación con ella.
inslv::datos::Ejemplos ejemplos() {
  return inslv::datos::Ejemplos(
      {{0, 1, 0, 1, 0, 1, 0, 1}, {0, 0, 1, 1, 0, 0, 1, 1}, {0, 1, 0, 1, 0, 1, 0, 1}});
}

inslv::genetico::MultiPoblacion poblacion() { return {2, 20, 4, 3, 0.5, 0, 0.01}; }

class Bondad : public testing::Test {
 protected:
  // La bondad de la regla con estos valores en la posición 0 de la población.
  double valora(const inslv::genetico::Codigo& code, inslv::genetico::Valoracion& valor) {
    G.poner_codigo(0, code);
    const inslv::aprendizaje::Evaluacion ev = evaluacion();
    return inslv::aprendizaje::bondad(ev, 0, valor, eliminables);
  }

  [[nodiscard]] inslv::aprendizaje::Evaluacion evaluacion() {
    return {G, V, E, R, false, por_clase, tabla, basica, errores, 0};
  }

  inslv::difuso::Variables V = variables();
  inslv::datos::Ejemplos E = ejemplos();
  inslv::aprendizaje::TablaDeAdaptaciones tabla{V, E};
  inslv::genetico::MultiPoblacion G = poblacion();
  inslv::reglas::ConjuntoDeReglas R;
  std::vector<int> por_clase = E.sin_clasificar_bien_por_clase(2, 2);
  std::vector<int> basica = std::vector<int>(1);
  std::vector<int> errores = std::vector<int>(1);
  std::vector<int> eliminables = {7};
};

TEST(Multipoblacion, ElCodigoEsElDelIndividuo) {
  inslv::genetico::MultiPoblacion G = poblacion();
  const inslv::genetico::Codigo code{"0110", 1, {0.1, 0.2, 0.3}};
  G.poner_codigo(5, code);
  EXPECT_EQ(G.etiquetas(5), "0110");
  EXPECT_EQ(G.clase(5), 1);
  EXPECT_EQ(G.niveles(5), (std::vector<double>{0.1, 0.2, 0.3}));
  const inslv::genetico::Codigo copia = G.codigo(5);
  EXPECT_EQ(copia.etiquetas, code.etiquetas);
  EXPECT_EQ(copia.clase, code.clase);
  EXPECT_EQ(copia.niveles, code.niveles);
}

TEST(Multipoblacion, OrdenaPorValoracionSinCambiarElOrdenDeLosIguales) {
  inslv::genetico::MultiPoblacion G(2, 4, 4, 3, 0.5, 0, 0.01);
  // Cada individuo se distingue por su clase y su primer bit.
  const std::vector<inslv::genetico::Codigo> codigos = {{"0000", 0, {0, 0, 0}},
                                                        {"1000", 0, {0, 0, 0}},
                                                        {"0000", 1, {0, 0, 0}},
                                                        {"1000", 1, {0, 0, 0}}};
  const std::vector<double> valor = {1, 3, 1, 2};
  for (std::size_t i = 0; i < codigos.size(); ++i) {
    G.poner_codigo(static_cast<int>(i), codigos[i]);
    G.valorar(static_cast<int>(i), {valor[i]});
  }

  G.ordenar();

  EXPECT_EQ(G.valoracion(0)[0], 3);
  EXPECT_EQ(G.etiquetas(0), "1000");
  EXPECT_EQ(G.clase(0), 0);
  EXPECT_EQ(G.valoracion(1)[0], 2);
  EXPECT_EQ(G.valoracion(2)[0], 1);
  EXPECT_EQ(G.clase(2), 0);
  EXPECT_EQ(G.valoracion(3)[0], 1);
  EXPECT_EQ(G.clase(3), 1);
}

TEST(Multipoblacion, ValorarQuitaLaMarcaDeModificado) {
  inslv::genetico::MultiPoblacion G = poblacion();
  EXPECT_TRUE(G.modificado(0));
  G.valorar(0, {1});
  EXPECT_FALSE(G.modificado(0));
  G.marcar_modificado(0);
  EXPECT_TRUE(G.modificado(0));
}

TEST_F(Bondad, ElPesoDeUnIndividuo) {
  double aciertos = 0;
  double fallos = 0;
  // Si a es True, Si: acierta los cuatro ejemplos que cubre.
  G.poner_codigo(0, {"0111", 1, {1, 0, 0.5}});
  EXPECT_EQ(inslv::aprendizaje::peso_del_individuo(G, E, tabla, 0, aciertos, fallos), 1);
  EXPECT_EQ(aciertos, 4);
  EXPECT_EQ(fallos, 0);
  // Si a es True, No: los falla.
  G.poner_codigo(0, {"0111", 0, {1, 0, 0.5}});
  EXPECT_DOUBLE_EQ(inslv::aprendizaje::peso_del_individuo(G, E, tabla, 0, aciertos, fallos),
                   1.0 / 5.0);
  EXPECT_EQ(aciertos, 0);
  EXPECT_EQ(fallos, 4);
}

TEST_F(Bondad, UnaReglaSinAtributosNoEsValida) {
  inslv::genetico::Valoracion valor{};
  EXPECT_EQ(valora({"1111", 1, {0, 0, 0.5}}, valor), 0);
  for (const double v : valor) {
    EXPECT_EQ(v, -16);
  }
  // Si la regla no es válida, las reglas a eliminar no cambian.
  EXPECT_EQ(eliminables, std::vector<int>{7});
}

TEST_F(Bondad, UnaReglaQueNoAdmiteNingunaEtiquetaNoEsValida) {
  inslv::genetico::Valoracion valor{};
  EXPECT_EQ(valora({"0011", 1, {1, 0, 0.5}}, valor), 0);
  EXPECT_EQ(valor[0], -16);
}

TEST_F(Bondad, UnAtributoQueAdmiteTodasSusEtiquetasNoEsRelevante) {
  inslv::genetico::Valoracion valor{};
  valora({"0111", 1, {1, 1, 0.5}}, valor);
  // b admite todas: le baja el nivel por debajo del umbral.
  EXPECT_LT(G.niveles(0)[1], 0.5);
  EXPECT_EQ(G.niveles(0)[0], 1);
}

TEST_F(Bondad, UnaReglaQueSoloAciertaEsMejorQueUnaQueTambienFalla) {
  inslv::genetico::Valoracion acierta{};
  inslv::genetico::Valoracion falla{};
  EXPECT_EQ(valora({"0111", 1, {1, 0, 0.5}}, acierta), 1);
  EXPECT_DOUBLE_EQ(valora({"0111", 0, {1, 0, 0.5}}, falla), 1.0 / 5.0);
  EXPECT_GT(acierta[0], falla[0]);
  // Sin más reglas, nada que eliminar.
  EXPECT_TRUE(eliminables.empty());
}

TEST_F(Bondad, GeneralizarQuitaElAtributoQueNoHaceFalta) {
  // Si a es True y b es True, Si: sin b acierta el doble.
  G.poner_codigo(0, {"0101", 1, {1, 1, 0.5}});
  const inslv::aprendizaje::Evaluacion ev = evaluacion();
  inslv::aprendizaje::generalizar(ev, 0, eliminables);
  EXPECT_EQ(G.niveles(0)[0], 1);
  EXPECT_LT(G.niveles(0)[1], 0.5);
  EXPECT_FALSE(G.modificado(0));
}

TEST_F(Bondad, EspecializarQuitaLasEtiquetasQueNoHacenFalta) {
  // Si a es False o True, Si: acierta la mitad y falla la otra; con a en True solo acierta.
  G.poner_codigo(0, {"1111", 1, {1, 0, 0.5}});
  const inslv::aprendizaje::Evaluacion ev = evaluacion();
  inslv::aprendizaje::especializar(ev, 0, eliminables);
  EXPECT_EQ(G.etiquetas(0).substr(0, 2), "01");
  EXPECT_FALSE(G.modificado(0));
}

TEST(Aciertos, AnotaLoQueDaCadaEjemplo) {
  const inslv::difuso::Variables V = variables();
  inslv::datos::Ejemplos E = ejemplos();
  inslv::reglas::ConjuntoDeReglas R;
  R.poner_variables(V);
  // Si a es True, Si: acierta los cuatro con a a 1 y no dice nada de los otros.
  R.anadir({"0111", 1, {1, 0, 0.5}}, 0.5);

  EXPECT_EQ(inslv::aprendizaje::aciertos(R, V, E), 0.5);

  EXPECT_EQ(E.sin_clasificar_bien(), 4);
  EXPECT_FALSE(E.bien_clasificado(0));
  EXPECT_EQ(E.regla_usada(0), -1);
  EXPECT_TRUE(E.bien_clasificado(1));
  EXPECT_EQ(E.regla_usada(1), 0);
  EXPECT_EQ(E.grado_positivo(1), 0.5);
}

TEST(Aciertos, LasReglasQueDisparan) {
  const inslv::difuso::Variables V = variables();
  const inslv::datos::Ejemplos E = ejemplos();
  inslv::reglas::ConjuntoDeReglas R;
  R.poner_variables(V);
  R.anadir({"0111", 1, {1, 0, 0.5}}, 0.5);   // Cubre la mitad.
  R.anadir({"0111", 0, {1, 0, 0.5}}, 0.25);  // Cubre lo mismo, con menos peso: nunca gana.
  R.anadir({"1011", 0, {1, 0, 0.5}}, 0.5);   // Cubre la otra mitad.
  EXPECT_EQ(inslv::aprendizaje::reglas_que_disparan(R, E), (std::vector<int>{0, 2}));
}

}  // namespace
