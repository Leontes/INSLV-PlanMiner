// SPDX-License-Identifier: GPL-3.0-only
// La población del genético: varias poblaciones con los mismos individuos.
#pragma once

#include <vector>

#include "inslv/genetico/genetcode.h"
#include "inslv/genetico/poblacionbinaria.h"
#include "inslv/genetico/poblacionentera.h"
#include "inslv/genetico/poblacionreal.h"

namespace inslv {

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
class multipoblacion {
 public:
  /// n individuos de clases clases, con tama_valor bits en el nivel de valor y tama_nivel valores
  /// en el de variable, con esas probabilidades de mutación. Cada individuo, con n_val valores de
  /// fitness.
  multipoblacion(int clases, int n, int tama_valor, int tama_nivel, double mut_valor,
                 double mut_clase, double mut_nivel, int n_val);

  void Swap(int i, int j);
  /// Copia el individuo j en el i, sin su valoración.
  void Pasar(int i, int j);

  /// La clase del individuo: su consecuente.
  [[nodiscard]] int ClaseIndividuo(int ind) const;
  /// El nivel de valor o el de variable del individuo (se pueden cambiar) y su tamaño, en tama.
  char* Subpoblacion_Binaria(int individuo, int& tama);
  double* Subpoblacion_Real(int individuo, int& tama);

  /// Ordena de mayor a menor valoración (burbuja: los iguales no cambian de orden).
  void Sort();
  /// Igual, salvo los dos últimos.
  void Sort_menos2ultimos();
  /// Coloca los dos últimos en su sitio. Si uno de los dos es de una clase con pocos individuos
  /// (sin contar los dos últimos), lo cambia por el último de una clase con más, para que no se
  /// pierda.
  void Sort_4L();

  [[nodiscard]] int N_individuos() const;
  void Codigo(int i, genetcode& code) const;
  void PutCodigo(int i, const genetcode& code);
  /// Si ha cambiado desde que se valoró.
  [[nodiscard]] bool Modificado(int i) const;
  void PutModificado(int i);
  [[nodiscard]] int N_Valoracion() const;
  /// Valora el individuo con los N_Valoracion() valores de dato.
  void Valoracion(int i, const double* dato);
  [[nodiscard]] double ValorFitness(int i) const;
  [[nodiscard]] double ValorFitness(int i, int j) const;

  /// Inicializa las poblaciones: el nivel de valor, con sujetos (por variable, la etiqueta de un
  /// ejemplo, o -1); las clases, por turnos; el nivel de variable, con I
  /// (poblacionreal::PoblacionInicial).
  void PoblacionInicial(const double* const* I, const int* const* sujetos, int n_var,
                        const int* n_etiquetas_variable);

  /// Con probabilidad 0,25 muta el nivel de variable de los dos últimos.
  void MutacionUniforme_Estacionario_Especial();
  /// Muta los dos últimos: cada población, con su probabilidad, hasta que le toque a alguna.
  void MutacionUniforme_Estacionario();
  /// Copia en los dos últimos dos individuos al azar (con probabilidad it, el primero y otro) y
  /// los cruza en dos puntos en las poblaciones que toque al azar, al menos en una.
  void CruceBasedLogical_Estacionario(double it);

 private:
  int n_individuos_;
  int n_clases_;
  int n_valoracion_;
  std::vector<bool> modificado_;
  std::vector<std::vector<double>> valoracion_;
  poblacionbinaria valor_;
  poblacionentera clase_;
  poblacionreal nivel_;
};

}  // namespace inslv
