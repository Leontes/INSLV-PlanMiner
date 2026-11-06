// SPDX-License-Identifier: GPL-3.0-only
// El generador: la secuencia de srand y rand de glibc, sin estado global.

#include <gtest/gtest.h>

#include <array>
#include <climits>
#include <cstdlib>

#include "inslv/generador_de_azar.hpp"

namespace {

TEST(GeneradorDeAzar, LaSecuenciaConocidaDeGlibc) {
  // Los primeros números de rand() en un programa sin srand(), en cualquier sistema con glibc.
  inslv::GeneradorDeAzar azar;
  for (const int esperado : {1804289383, 846930886, 1681692777, 1714636915, 1957747793}) {
    EXPECT_EQ(azar(), esperado);
  }
}

TEST(GeneradorDeAzar, LaSemillaCeroEsComoLaUno) {
  inslv::GeneradorDeAzar cero(0);
  inslv::GeneradorDeAzar uno(1);
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(cero(), uno());
  }
}

TEST(GeneradorDeAzar, SembrarVuelveAEmpezar) {
  inslv::GeneradorDeAzar azar(7);
  const int primero = azar();
  azar();
  azar.sembrar(7);
  EXPECT_EQ(azar(), primero);
}

TEST(GeneradorDeAzar, DosGeneradoresNoSeMolestan) {
  inslv::GeneradorDeAzar uno(5);
  inslv::GeneradorDeAzar copia(5);
  inslv::GeneradorDeAzar otro(6);
  for (int i = 0; i < 100; ++i) {
    otro();
    EXPECT_EQ(uno(), copia());
  }
}

TEST(GeneradorDeAzar, NoSeSaleDelRango) {
  inslv::GeneradorDeAzar azar(3);
  for (int i = 0; i < 100000; ++i) {
    const int numero = azar();
    EXPECT_GE(numero, 0);
    EXPECT_LE(numero, inslv::GeneradorDeAzar::maximo);
  }
}

#ifdef __GLIBC__
TEST(GeneradorDeAzar, LaMismaSecuenciaQueRand) {
  EXPECT_EQ(inslv::GeneradorDeAzar::maximo, RAND_MAX);
  // Entre ellas, semillas que pasan de INT_MAX, que glibc trata con signo.
  const std::array<unsigned, 8> semillas = {
      0, 1, 2, 42, 12345, static_cast<unsigned>(INT_MAX), 2147483648U, UINT_MAX};
  for (const unsigned semilla : semillas) {
    std::srand(semilla);  // NOLINT(concurrency-mt-unsafe)
    inslv::GeneradorDeAzar azar(semilla);
    for (int i = 0; i < 100000; ++i) {
      ASSERT_EQ(azar(), std::rand()) << semilla << ", " << i;  // NOLINT(concurrency-mt-unsafe)
    }
  }
}
#endif

}  // namespace
