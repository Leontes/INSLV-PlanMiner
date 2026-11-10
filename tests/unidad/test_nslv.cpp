// SPDX-License-Identifier: GPL-3.0-only
// La lectura del formato de NSLV (inslv/formatos/nslv.hpp), con los conjuntos de tests/datos.

#include <gtest/gtest.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

#include "inslv/formatos/nslv.hpp"
#include "inslv/inslv.hpp"
#include "keel.hpp"

namespace {

namespace fs = std::filesystem;
using inslv::nslv::ErrorDeFormato;

const fs::path datos_nslv = fs::path(INSLV_DATOS) / "nslv";

// Las particiones de un conjunto: <nombre>0.datos a <nombre>9.datos.
std::vector<fs::path> particiones(const std::string& nombre) {
  std::vector<fs::path> ficheros;
  ficheros.reserve(10);
  for (int i = 0; i < 10; ++i) {
    ficheros.push_back(datos_nslv / nombre / (nombre + std::to_string(i) + ".datos"));
  }
  return ficheros;
}

inslv::Datos leer_conjunto(const std::string& nombre, const std::vector<fs::path>& ejemplos) {
  return inslv::nslv::leer(datos_nslv / nombre / (nombre + ".dom"), ejemplos);
}

// Los ejemplos como filas (atributos y la clase al final), ordenadas, para comparar conjuntos sin
// tener en cuenta el orden. Los desconocidos pasan a -infinito para poder compararlos.
std::vector<std::vector<double>> filas(const inslv::Datos& datos) {
  std::vector<std::vector<double>> resultado(datos.clases.size());
  for (std::size_t e = 0; e < datos.clases.size(); ++e) {
    for (const std::vector<double>& columna : datos.valores) {
      resultado[e].push_back(std::isnan(columna[e]) ? -std::numeric_limits<double>::infinity()
                                                    : columna[e]);
    }
    resultado[e].push_back(static_cast<double>(datos.clases[e]));
  }
  std::sort(resultado.begin(), resultado.end());
  return resultado;
}

// Un fichero temporal con el contenido, con el nombre del test para que no choquen los tests que
// corren a la vez.
fs::path escribir(const std::string& nombre, const std::string& contenido) {
  const testing::TestInfo* const test = testing::UnitTest::GetInstance()->current_test_info();
  const fs::path ruta = fs::path(testing::TempDir()) /
                        (std::string(test->test_suite_name()) + "." + test->name() + "." + nombre);
  std::ofstream(ruta) << contenido;
  return ruta;
}

TEST(Nslv, LeeElDominio) {
  const inslv::Datos datos = inslv::nslv::leer_dominio(datos_nslv / "irs" / "irs.dom");

  ASSERT_EQ(datos.atributos.size(), 4U);
  EXPECT_EQ(datos.atributos[0].nombre, "Variable0");
  ASSERT_EQ(datos.atributos[0].etiquetas.size(), 5U);
  const inslv::ConjuntoDifuso& primera = datos.atributos[0].etiquetas[0];
  EXPECT_EQ(primera.etiqueta, "MuyBaja");
  EXPECT_EQ(primera.a, 4.0);
  EXPECT_EQ(primera.b, 4.0);
  EXPECT_EQ(primera.c, 4.0);
  EXPECT_EQ(primera.d, 5.0);

  EXPECT_EQ(datos.clase.nombre, "Clase");
  ASSERT_EQ(datos.clase.etiquetas.size(), 3U);
  EXPECT_EQ(datos.clase.etiquetas[2].etiqueta, "Iris-virginica");

  EXPECT_EQ(datos.valores.size(), 4U);
  EXPECT_TRUE(datos.valores[0].empty());
  EXPECT_TRUE(datos.clases.empty());
}

TEST(Nslv, LeeLaUnidadDeCadaVariable) {
  const inslv::Datos datos = inslv::nslv::leer_dominio(datos_nslv / "wne" / "wne.dom");
  ASSERT_GE(datos.atributos.size(), 3U);
  EXPECT_EQ(datos.atributos[0].unidad, 0U);
  EXPECT_EQ(datos.atributos[1].unidad, 1U);
  EXPECT_EQ(datos.atributos[2].unidad, 2U);
  for (const inslv::Variable& atributo :
       inslv::nslv::leer_dominio(datos_nslv / "boxA" / "boxA.dom").atributos) {
    EXPECT_EQ(atributo.unidad, 1U) << atributo.nombre;
  }
}

TEST(Nslv, LeeLosEjemplos) {
  const inslv::Datos datos = leer_conjunto("irs", {datos_nslv / "irs" / "irs0.datos"});

  ASSERT_EQ(datos.clases.size(), 15U);
  // La primera fila de irs0.datos: 5.8 2.8 5.1 2.4 2.
  EXPECT_EQ(datos.valores[0][0], 5.8);
  EXPECT_EQ(datos.valores[1][0], 2.8);
  EXPECT_EQ(datos.valores[2][0], 5.1);
  EXPECT_EQ(datos.valores[3][0], 2.4);
  EXPECT_EQ(datos.clases[0], 2U);
}

struct Tamano {
  std::string nombre;
  std::size_t atributos;
  std::size_t clases;
  std::size_t ejemplos;
};

class Tamanos : public testing::TestWithParam<Tamano> {};

TEST_P(Tamanos, LasDiezParticiones) {
  const Tamano& esperado = GetParam();
  const inslv::Datos datos = leer_conjunto(esperado.nombre, particiones(esperado.nombre));

  EXPECT_EQ(datos.atributos.size(), esperado.atributos);
  EXPECT_EQ(datos.clase.etiquetas.size(), esperado.clases);
  EXPECT_EQ(datos.clases.size(), esperado.ejemplos);
  for (const std::vector<double>& columna : datos.valores) {
    EXPECT_EQ(columna.size(), esperado.ejemplos);
  }
}

INSTANTIATE_TEST_SUITE_P(Conjuntos, Tamanos,
                         testing::Values(Tamano{"aut", 25, 6, 205}, Tamano{"boxA", 6, 3, 1000},
                                         Tamano{"irs", 4, 3, 150}, Tamano{"ring", 20, 2, 7400},
                                         Tamano{"wne", 13, 3, 178}),
                         [](const testing::TestParamInfo<Tamano>& tamano) {
                           return tamano.param.nombre;
                         });

TEST(Nslv, LosDesconocidosSonNaN) {
  const inslv::Datos datos = leer_conjunto("aut", {datos_nslv / "aut" / "aut.datos"});

  // La primera fila de aut.datos empieza por -999999999 (normalized-loses).
  EXPECT_TRUE(std::isnan(datos.valores[0][0]));
  const auto desconocidos = std::count_if(datos.valores[0].begin(), datos.valores[0].end(),
                                          [](double valor) { return std::isnan(valor); });
  EXPECT_GT(desconocidos, 0);
  EXPECT_LT(desconocidos, static_cast<std::ptrdiff_t>(datos.valores[0].size()));
}

TEST(Nslv, LasParticionesDeAutSonElConjuntoCompleto) {
  EXPECT_EQ(filas(leer_conjunto("aut", particiones("aut"))),
            filas(leer_conjunto("aut", {datos_nslv / "aut" / "aut.datos"})));
}

// Un nombre de atributo de aut, para emparejar los de KEEL con los de NSLV: NSLV cambia algunos
// guiones por guiones bajos (fuel_type, pero wheel-base) y escribe normalized-loses.
std::string normalizar(std::string nombre) {
  std::replace(nombre.begin(), nombre.end(), '-', '_');
  return nombre == "normalized_losses" ? "normalized_loses" : nombre;
}

double numero(const std::string& texto) {
  double valor = 0;
  std::from_chars(texto.data(), texto.data() + texto.size(), valor);
  return valor;
}

// Las particiones de aut de KEEL son otra partición de los mismos ejemplos, escritos de otra
// forma: emparejando atributos y etiquetas por nombre, tienen que salir los mismos.
TEST(Nslv, MismosEjemplosQueKeel) {
  std::vector<fs::path> ficheros_keel;
  ficheros_keel.reserve(10);
  for (int i = 0; i < 10; ++i) {
    ficheros_keel.push_back(fs::path(INSLV_DATOS) / "keel" / "aut" /
                            ("aut-10-" + std::to_string(i) + "tst.dat"));
  }
  const inslv::pruebas::DatosKeel keel = inslv::pruebas::leer_keel(ficheros_keel);
  const inslv::Datos nslv = leer_conjunto("aut", particiones("aut"));
  ASSERT_EQ(keel.atributos.size(), nslv.atributos.size() + 1);
  ASSERT_EQ(keel.atributos.back().nombre, nslv.clase.nombre);

  // La columna de KEEL de cada atributo de NSLV.
  std::vector<std::size_t> columna;
  for (const inslv::Variable& atributo : nslv.atributos) {
    const auto encontrado = std::find_if(
        keel.atributos.begin(), keel.atributos.end(),
        [&](const auto& otro) { return normalizar(otro.nombre) == normalizar(atributo.nombre); });
    ASSERT_NE(encontrado, keel.atributos.end()) << atributo.nombre;
    columna.push_back(static_cast<std::size_t>(std::distance(keel.atributos.begin(), encontrado)));
  }

  inslv::Datos traducido = nslv;
  traducido.valores.assign(nslv.atributos.size(), {});
  traducido.clases.clear();
  const std::vector<std::string>& clases_keel = keel.atributos.back().valores;
  for (const std::vector<std::string>& ejemplo : keel.ejemplos) {
    for (std::size_t i = 0; i < nslv.atributos.size(); ++i) {
      const std::string& valor = ejemplo[columna[i]];
      double traduccion = numero(valor);
      if (valor == "?") {
        traduccion = std::numeric_limits<double>::quiet_NaN();
      } else if (!keel.atributos[columna[i]].valores.empty()) {
        // Nominal: el centro de la etiqueta de NSLV que se llama como el valor.
        const std::vector<inslv::ConjuntoDifuso>& etiquetas = nslv.atributos[i].etiquetas;
        const auto etiqueta =
            std::find_if(etiquetas.begin(), etiquetas.end(),
                         [&](const auto& otra) { return otra.etiqueta == valor; });
        ASSERT_NE(etiqueta, etiquetas.end()) << nslv.atributos[i].nombre << " = " << valor;
        traduccion = (etiqueta->b + etiqueta->c) / 2.0;
      }
      traducido.valores[i].push_back(traduccion);
    }
    // La clase: value<i> en NSLV es el i-ésimo valor que declara KEEL.
    const auto clase = std::find(clases_keel.begin(), clases_keel.end(), ejemplo.back());
    ASSERT_NE(clase, clases_keel.end()) << ejemplo.back();
    traducido.clases.push_back(static_cast<std::size_t>(std::distance(clases_keel.begin(), clase)));
  }

  EXPECT_EQ(filas(traducido), filas(nslv));
}

TEST(Nslv, LosDatosSirvenParaAprender) {
  const std::vector<inslv::Regla> reglas =
      inslv::aprender(leer_conjunto("irs", particiones("irs")));
  EXPECT_FALSE(reglas.empty());
}

// Un dominio pequeño y correcto, para estropearlo en los tests de errores.
const std::string dominio_valido =
    "2\n"
    "a 0 0 1 0 1 2\n0 0 0 1 Bajo\n0 1 1 1 Alto\n"
    "Clase 1 0 1 0 1 2\n0 0 0 0 No\n1 1 1 1 Si\n";

// Que lea mal lance ErrorDeFormato con el fichero y la línea, y un mensaje que lo explique.
void espera_error(const std::function<void()>& leer, const std::string& fichero, int linea,
                  const std::string& texto) {
  try {
    leer();
    ADD_FAILURE() << "no lanza ErrorDeFormato; se esperaba: " << texto;
  } catch (const ErrorDeFormato& error) {
    const std::string mensaje = error.what();
    EXPECT_NE(mensaje.find(fichero + ":" + std::to_string(linea) + ":"), std::string::npos)
        << mensaje;
    EXPECT_NE(mensaje.find(texto), std::string::npos) << mensaje;
  }
}

TEST(Nslv, ErroresDelDominio) {
  const auto dominio = [](const std::string& contenido) {
    const fs::path fichero = escribir("dom", contenido);
    return [fichero] { inslv::nslv::leer_dominio(fichero); };
  };
  const std::string fichero =
      (fs::path(testing::TempDir()) / "Nslv.ErroresDelDominio.dom").string();

  espera_error(dominio("2\na 0 0 1 0 1 1\n0 0 0 1 Bajo\nb 0 0 1 0 1 1\n0 0 0 1 Bajo\n"), fichero, 5,
               "ninguna variable es la clase");
  espera_error(dominio("2\na 1 0 1 0 1 1\n0 0 0 1 Bajo\nb 1 0 1 0 1 1\n0 0 0 1 Bajo\n"), fichero, 4,
               "hay dos clases: a y b");
  espera_error(dominio("1\nClase 1 0 1 0 1 1\n0 1 0 1 Mal\n"), fichero, 3,
               "la etiqueta Mal de Clase no cumple a <= b <= c <= d");
  espera_error(dominio("1\nClase 1 0 1 0 uno 1\n0 0 0 0 No\n"), fichero, 2,
               "el máximo de Clase no es un número: uno");
  espera_error(dominio("1\nClase 1 0 1 0 1 0\n"), fichero, 2, "Clase no tiene etiquetas");
  espera_error(dominio("1\nClase 1 1.5 1 0 1 1\n0 0 0 0 No\n"), fichero, 2,
               "la unidad de Clase no es un número natural: 1.5");
  espera_error(dominio("1\nClase 2 0 1 0 1 1\n0 0 0 0 No\n"), fichero, 2,
               "si Clase es la clase tiene que ser 0 o 1");
  espera_error(dominio("1\nClase 1 0 1 0 1 2\n0 0 0 0 No\n"), fichero, 3,
               "se acaba el fichero y falta a de la etiqueta 1 de Clase");
  espera_error(dominio(dominio_valido + "sobra\n"), fichero, 8, "sobra texto al final: sobra");

  EXPECT_THROW(inslv::nslv::leer_dominio(fs::path(testing::TempDir()) / "no-existe.dom"),
               ErrorDeFormato);
}

TEST(Nslv, ErroresDeLosEjemplos) {
  const fs::path dominio = escribir("dom", dominio_valido);
  const auto ejemplos = [&](const std::string& contenido) {
    const fs::path fichero = escribir("datos", contenido);
    return [dominio, fichero] { inslv::nslv::leer(dominio, {fichero}); };
  };
  const std::string fichero =
      (fs::path(testing::TempDir()) / "Nslv.ErroresDeLosEjemplos.datos").string();

  EXPECT_NO_THROW(ejemplos("2\n2\n0.5 1\n-999999999 0\n")());
  espera_error(ejemplos("1\n3\n0.5 1 1\n"), fichero, 2, "hay 3 variables y el dominio tiene 2");
  espera_error(ejemplos("1\n2\n0.5 0.5\n"), fichero, 3,
               "la clase del ejemplo 0 (0.500000) no es el centro de ninguna etiqueta de Clase");
  espera_error(ejemplos("2\n2\n0.5 1\n"), fichero, 3,
               "se acaba el fichero y falta el valor 0 del ejemplo 1");
  espera_error(ejemplos("1\n2\n0.5 1 7\n"), fichero, 3, "sobra texto al final: 7");
}

}  // namespace
