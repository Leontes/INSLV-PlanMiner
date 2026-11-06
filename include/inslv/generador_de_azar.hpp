// SPDX-License-Identifier: GPL-3.0-only
// El generador de números aleatorios de INSLV.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace inslv {

/// Da la misma secuencia que srand y rand de glibc con la misma semilla, en cualquier plataforma y
/// sin estado global: las reglas no dependen de la biblioteca de C, no se toca el generador del
/// programa y cada hilo puede tener el suyo. Es el generador aditivo de glibc (TYPE_3, con r[i] =
/// r[i - 3] + r[i - 31]), descrito en la documentación de random(3).
class GeneradorDeAzar {
 public:
  /// El mayor número que da (el RAND_MAX de glibc).
  static constexpr int maximo = 2147483647;

  /// Sin semilla, la 1: como rand() si no se ha llamado a srand().
  explicit GeneradorDeAzar(unsigned semilla = 1) { sembrar(semilla); }

  /// Vuelve a empezar la secuencia de esa semilla (0 es lo mismo que 1).
  void sembrar(unsigned semilla) {
    // El valor de cada posición se calcula con signo, como glibc: de ahí la semilla negativa.
    std::int64_t valor = static_cast<std::int32_t>(semilla == 0 ? 1 : semilla);
    estado_[0] = static_cast<std::uint32_t>(valor);
    for (std::size_t i = 1; i < tamano_inicial; ++i) {
      valor = (16807 * valor) % maximo;
      if (valor < 0) {
        valor += maximo;
      }
      estado_[i] = static_cast<std::uint32_t>(valor);
    }
    for (std::size_t i = tamano_inicial; i < tamano; ++i) {
      estado_[i] = estado_[i - tamano_inicial];
    }
    posicion_ = 0;
    for (int i = 0; i < descartados; ++i) {
      siguiente();
    }
  }

  /// Un número de 0 a maximo.
  int operator()() { return static_cast<int>(siguiente() >> 1U); }

 private:
  static constexpr std::size_t tamano_inicial = 31;
  // Los últimos valores, entre los que están r[i - 31] y r[i - 3]. Al sembrar, los tres de más
  // repiten los primeros, como glibc.
  static constexpr std::size_t tamano = tamano_inicial + 3;
  // glibc descarta 310 números antes del primero.
  static constexpr int descartados = 310;

  std::uint32_t siguiente() {
    const std::uint32_t valor = estado_[(posicion_ + tamano - tamano_inicial) % tamano] +
                                estado_[(posicion_ + tamano - 3) % tamano];
    estado_[posicion_] = valor;
    posicion_ = (posicion_ + 1) % tamano;
    return valor;
  }

  std::array<std::uint32_t, tamano> estado_{};
  std::size_t posicion_ = 0;
};

}  // namespace inslv
