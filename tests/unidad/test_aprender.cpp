// SPDX-License-Identifier: GPL-3.0-only
// La interfaz de inslv::aprender (antes test_learn.cpp, el de old/).

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "inslv/inslv.hpp"

namespace {

inslv::Variable logica(const std::string& nombre) {
  return {nombre, {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}};
}

// 40 ejemplos: la clase es el valor de a; b no tiene relación con ella.
inslv::Datos clase_es_a() {
  inslv::Datos datos;
  datos.atributos = {logica("a"), logica("b")};
  datos.clase = {"Clase", {{"Negativa", 0, 0, 0, 0}, {"Positiva", 1, 1, 1, 1}}};
  datos.valores.assign(2, {});
  for (std::size_t e = 0; e < 40; ++e) {
    const std::size_t a = e % 2;
    datos.valores[0].push_back(static_cast<double>(a));
    datos.valores[1].push_back(static_cast<double>((e / 2) % 2));
    datos.clases.push_back(a);
  }
  return datos;
}

std::string describir(const std::vector<inslv::Regla>& reglas) {
  std::ostringstream salida;
  for (const inslv::Regla& regla : reglas) {
    salida << "clase " << regla.consecuente << " (" << regla.cubiertos << "):";
    for (const inslv::Condicion& condicion : regla.antecedente) {
      salida << " atributo " << condicion.atributo << " en {";
      for (const std::size_t etiqueta : condicion.etiquetas) {
        salida << ' ' << etiqueta;
      }
      salida << " }";
    }
    salida << '\n';
  }
  return salida.str();
}

TEST(Aprender, EncuentraElAtributoQueDecideLaClase) {
  const std::vector<inslv::Regla> reglas = inslv::aprender(clase_es_a());

  ASSERT_EQ(reglas.size(), 2U) << describir(reglas);
  for (const inslv::Regla& regla : reglas) {
    ASSERT_EQ(regla.antecedente.size(), 1U) << describir(reglas);
    EXPECT_EQ(regla.antecedente[0].atributo, 0U) << describir(reglas);
    EXPECT_EQ(regla.antecedente[0].etiquetas, std::vector<std::size_t>{regla.consecuente})
        << describir(reglas);
    EXPECT_EQ(regla.cubiertos, 20.0) << describir(reglas);
  }
}

TEST(Aprender, MismaSemillaMismasReglas) {
  const inslv::Datos datos = clase_es_a();
  EXPECT_EQ(describir(inslv::aprender(datos)), describir(inslv::aprender(datos)));
}

TEST(Aprender, NoTocaElGeneradorDelPrograma) {
  const inslv::Datos datos = clase_es_a();
  std::srand(123);                       // NOLINT(concurrency-mt-unsafe)
  const int sin_aprender = std::rand();  // NOLINT(concurrency-mt-unsafe)
  std::srand(123);                       // NOLINT(concurrency-mt-unsafe)
  static_cast<void>(inslv::aprender(datos));
  EXPECT_EQ(std::rand(), sin_aprender);  // NOLINT(concurrency-mt-unsafe)
}

TEST(Aprender, SeSiembraElGeneradorDeLasOpciones) {
  const inslv::Datos datos = clase_es_a();
  const std::string sin_generador = describir(inslv::aprender(datos));

  // Da igual en qué estado venga: se siembra con la semilla de las opciones.
  inslv::GeneradorDeAzar azar(99);
  azar();
  inslv::Opciones opciones;
  opciones.azar = &azar;
  EXPECT_EQ(describir(inslv::aprender(datos, opciones)), sin_generador);
  const int siguiente = azar();

  // Y se queda donde acabó el aprendizaje, igual cada vez.
  EXPECT_EQ(describir(inslv::aprender(datos, opciones)), sin_generador);
  EXPECT_EQ(azar(), siguiente);
}

TEST(Aprender, DesdeVariosHilosDaLasMismasReglas) {
  const inslv::Datos datos = clase_es_a();
  const std::string esperado = describir(inslv::aprender(datos));

  std::vector<std::string> resultados(8);
  std::vector<std::thread> hilos;
  hilos.reserve(resultados.size());
  for (std::string& resultado : resultados) {
    hilos.emplace_back([&datos, &resultado] { resultado = describir(inslv::aprender(datos)); });
  }
  for (std::thread& hilo : hilos) {
    hilo.join();
  }
  for (const std::string& resultado : resultados) {
    EXPECT_EQ(resultado, esperado);
  }
}

TEST(Aprender, AdmiteValoresDesconocidos) {
  inslv::Datos datos = clase_es_a();
  datos.valores[1][0] = std::numeric_limits<double>::quiet_NaN();
  datos.valores[1][7] = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(inslv::aprender(datos).empty());
}

TEST(Aprender, EscribeElInformeSiSePide) {
  const std::filesystem::path informe =
      std::filesystem::path(testing::TempDir()) / "inslv-informe-de-prueba.res";
  std::filesystem::remove(informe);

  inslv::Opciones opciones;
  opciones.informe = informe;
  inslv::aprender(clase_es_a(), opciones);

  std::ifstream entrada(informe);
  std::string primera_linea;
  std::getline(entrada, primera_linea);
  EXPECT_EQ(primera_linea, "Experimento: " + informe.string());
  std::filesystem::remove(informe);
}

// El caso con el que el original reventaba (HALLAZGOS: barman y counters en PlanMiner): ninguna
// regla es posible, porque el único atributo tiene una sola etiqueta (la discretización de
// PlanMiner deja un solo cluster en total-cost). La primera vuelta del genético no añade ninguna, y
// el original leía la regla -1 de los patrones, que estaban vacíos. Volcado de PlanMiner (barman,
// 6 trazas completas del banco).
TEST(Aprender, SinNingunaReglaPosibleNoDaReglas) {
  inslv::Datos datos;
  const inslv::Variable coste{"value (total-cost)", {{"L_0", 37.9999, 38, 38.05, 39.0005}}};
  datos.atributos = {coste};
  datos.clase = {"Class", {{"PreState", 0, 0, 0, 0}, {"PostState", 1, 1, 1, 1}}};
  datos.valores = {{38, 39}};
  datos.clases = {0, 1};
  std::vector<inslv::Regla> reglas;
  EXPECT_NO_THROW(reglas = inslv::aprender(datos));
  EXPECT_TRUE(reglas.empty());
}

TEST(Aprender, RechazaDatosIncoherentes) {
  inslv::Datos datos = clase_es_a();
  datos.valores.pop_back();
  EXPECT_THROW(inslv::aprender(datos), std::invalid_argument);

  datos = clase_es_a();
  datos.valores[1].pop_back();
  EXPECT_THROW(inslv::aprender(datos), std::invalid_argument);

  datos = clase_es_a();
  datos.clases[3] = 2;
  EXPECT_THROW(inslv::aprender(datos), std::invalid_argument);

  datos = clase_es_a();
  datos.atributos[0].etiquetas.clear();
  EXPECT_THROW(inslv::aprender(datos), std::invalid_argument);

  datos = clase_es_a();
  datos.clases.clear();
  datos.valores = {{}, {}};
  EXPECT_THROW(inslv::aprender(datos), std::invalid_argument);
}

}  // namespace
