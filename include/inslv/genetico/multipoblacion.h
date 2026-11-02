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
/// los dos últimos individuos.
///
/// El aprendizaje usa dos poblaciones de cada tipo: binarias, el nivel de valor (las etiquetas
/// de cada atributo) y el de valor en funciones; enteras, el consecuente (la clase) y el nivel de
/// variable en funciones; reales, el nivel de variable (con su umbral) y los ejemplos cubiertos.
class multipoblacion {
 public:
  /// pbin poblaciones binarias, pent enteras y preal reales, por ese orden, de n individuos: la
  /// k, de individuos de tama[k] valores que mutan con probabilidad mut[k]; la entera j, con
  /// valores de 0 a rangoe[j] - 1. Cada individuo, con n_val valores de fitness.
  multipoblacion(int pbin, int pent, int preal, const int* rangoe, const double* mut, int n,
                 const int* tama, int n_val);

  void Swap(int i, int j);
  /// Copia el individuo j en el i, sin su valoración.
  void Pasar(int i, int j);

  /// La clase del individuo: su consecuente.
  [[nodiscard]] int ClaseIndividuo(int ind) const;
  /// La parte del individuo en esa población (se puede cambiar) y su tamaño, en tama.
  char* Subpoblacion_Binaria(int subpoblacion, int individuo, int& tama);
  int* Subpoblacion_Entera(int subpoblacion, int individuo, int& tama);
  double* Subpoblacion_Real(int subpoblacion, int individuo, int& tama);

  /// Ordena de mayor a menor valoración (burbuja: los iguales no cambian de orden).
  void Sort();
  /// Igual, salvo los dos últimos.
  void Sort_menos2ultimos();
  /// Coloca los dos últimos en su sitio y cuenta en ind_clase[clase][0] los individuos de cada
  /// clase, sin los dos últimos, y en ind_clase[clase][1...] cuáles son. Si uno de los dos
  /// últimos es de una clase con pocos individuos, lo cambia por el último de una con más, para
  /// que no se pierda.
  void Sort_4L(int n_clases, int** ind_clase);
  /// Los mejores individuos: siempre el primero.
  static void Better(int* better, int& nbetter);
  /// La valoración y la clase del individuo, a la depuración.
  void PintaFitness_Estacionario(int i) const;

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

  /// Inicializa las poblaciones del aprendizaje (ver la clase): el nivel de valor, con sujetos
  /// (por variable, la etiqueta de un ejemplo, o -1); el de valor en funciones, al azar; las
  /// clases, de una en una; el nivel de variable en funciones, a 0; el de variable, con I
  /// (poblacionreal::PoblacionInicial, con rango clases), y los ejemplos cubiertos, a n_items.
  void PoblacionInicial(const double* const* I, int rango, int n_items, const int* const* sujetos,
                        int n_var, const int* n_etiquetas_variable);

  /// Muta los dos últimos: con probabilidad 0,5, el nivel de variable en funciones y, si no, con
  /// otra de 0,5, el de variable.
  void MutacionUniforme_Estacionario_Especial();
  /// Muta los dos últimos: cada población, con su probabilidad, hasta que cambie al menos una.
  void MutacionUniforme_Estacionario();
  /// Copia en los dos últimos dos individuos al azar (con probabilidad it, el primero y otro) y
  /// los cruza en dos puntos en las poblaciones que toque al azar, al menos en una.
  void CruceBasedLogical_Estacionario(double it);

 private:
  int n_individuos_;
  int n_valoracion_;
  std::vector<bool> modificado_;
  std::vector<std::vector<double>> valoracion_;
  std::vector<poblacionbinaria> Pb_;
  std::vector<poblacionentera> Pe_;
  std::vector<poblacionreal> Pr_;
};

}  // namespace inslv
