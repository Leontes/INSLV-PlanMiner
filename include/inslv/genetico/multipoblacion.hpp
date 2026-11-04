// SPDX-License-Identifier: GPL-3.0-only
// La población del genético: varias poblaciones con los mismos individuos.
#pragma once

#include <array>
#include <string>
#include <vector>

#include "inslv/genetico/codigo.hpp"
#include "inslv/genetico/poblacion_binaria.hpp"
#include "inslv/genetico/poblacion_entera.hpp"
#include "inslv/genetico/poblacion_real.hpp"

namespace inslv::genetico {

/// La valoración de un individuo: sus valores se comparan en orden.
using Valoracion = std::array<double, 7>;

/// Cada individuo tiene una parte en cada población y una valoración (su fitness, de varios
/// valores que se comparan en orden). Es un genético estacionario: cada generación cambia solo
/// los dos últimos individuos. Las partes son el nivel de valor (qué etiquetas admite cada
/// atributo, en una población binaria), el consecuente (la clase, en una entera) y el nivel de
/// variable (el nivel de cada atributo y, al final, el umbral, en una real).
///
/// El original tenía además el nivel de valor en funciones (40 bits) y el nivel de variable en
/// funciones (20 enteros), para variables construidas a partir de otras que nunca se creaban, y una
/// población real de un valor que no se leía. Sus valores no influían en nada, pero mutar y cruzar
/// esas poblaciones pedía números al azar: hasta que haya un generador propio (fase B), se piden
/// los mismos, para que las reglas no cambien.
class MultiPoblacion {
 public:
  /// n individuos de clases clases, con tama_valor bits en el nivel de valor y tama_nivel valores
  /// en el de variable, con esas probabilidades de mutación.
  MultiPoblacion(int clases, int n, int tama_valor, int tama_nivel, double mut_valor,
                 double mut_clase, double mut_nivel);

  void intercambiar(int i, int j);
  /// Copia el individuo j en el i, sin su valoración.
  void copiar(int i, int j);

  /// La clase del individuo: su consecuente.
  [[nodiscard]] int clase(int ind) const;
  /// El nivel de valor y el de variable del individuo (se pueden cambiar).
  [[nodiscard]] std::string& etiquetas(int individuo);
  [[nodiscard]] const std::string& etiquetas(int individuo) const;
  [[nodiscard]] std::vector<double>& niveles(int individuo);
  [[nodiscard]] const std::vector<double>& niveles(int individuo) const;

  /// Ordena de mayor a menor valoración (burbuja: los iguales no cambian de orden).
  void ordenar();
  /// Igual, salvo los dos últimos.
  void ordenar_sin_los_dos_ultimos();
  /// Coloca los dos últimos en su sitio. Si uno de los dos es de una clase con pocos individuos
  /// (sin contar los dos últimos), lo cambia por el último de una clase con más, para que no se
  /// pierda.
  void colocar_los_dos_ultimos();

  [[nodiscard]] int numero_de_individuos() const;
  [[nodiscard]] Codigo codigo(int i) const;
  void poner_codigo(int i, const Codigo& code);
  /// Si ha cambiado desde que se valoró.
  [[nodiscard]] bool modificado(int i) const;
  void marcar_modificado(int i);
  /// Valora el individuo: deja de estar modificado.
  void valorar(int i, const Valoracion& valoracion);
  [[nodiscard]] const Valoracion& valoracion(int i) const;

  /// Inicializa las poblaciones: el nivel de valor, con sujetos (por variable, la etiqueta de un
  /// ejemplo, o -1); las clases, por turnos; el nivel de variable, con I
  /// (PoblacionReal::inicializar).
  void inicializar(const std::vector<std::vector<double>>& I,
                   const std::vector<std::vector<int>>& sujetos,
                   const std::vector<int>& n_etiquetas_variable);

  /// Con probabilidad 0,25 muta el nivel de variable de los dos últimos.
  void mutar_niveles();
  /// Muta los dos últimos: cada población, con su probabilidad, hasta que le toque a alguna.
  void mutar();
  /// Copia en los dos últimos dos individuos al azar (con probabilidad it, el primero y otro) y
  /// los cruza en dos puntos en las poblaciones que toque al azar, al menos en una.
  void cruzar(double it);

 private:
  int n_individuos_;
  int n_clases_;
  std::vector<bool> modificado_;
  std::vector<Valoracion> valoracion_;
  PoblacionBinaria etiquetas_;
  PoblacionEntera clases_;
  PoblacionReal niveles_;
};

}  // namespace inslv::genetico
