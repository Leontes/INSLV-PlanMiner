// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/multipoblacion.h"

#include <cstddef>
#include <ostream>
#include <span>
#include <utility>
#include <vector>

#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Si la valoración v1 es mejor que la v2: en el primer valor en el que no son iguales, el de v1
// es mayor.
bool Mayor(const std::vector<double>& v1, const std::vector<double>& v2) {
  std::size_t k = 0;
  while (k < v1.size() && v1[k] == v2[k]) {
    ++k;
  }
  return k < v1.size() && v1[k] > v2[k];
}

// Las partes de un individuo en unas poblaciones, para un genetcode.
template <typename Poblacion>
auto Partes(const std::vector<Poblacion>& poblaciones, int i) {
  using T = typename decltype(poblaciones[0].Individuo(i))::value_type;
  std::vector<std::vector<T>> partes;
  partes.reserve(poblaciones.size());
  for (const Poblacion& p : poblaciones) {
    const std::span<const T> parte = p.Individuo(i);
    partes.emplace_back(parte.begin(), parte.end());
  }
  return partes;
}

// La parte de un individuo en una población, para el código heredado.
template <typename Poblacion>
auto Parte(std::vector<Poblacion>& poblaciones, int subpoblacion, int individuo, int& tama) {
  const auto parte = poblaciones[indice(subpoblacion)].Individuo(individuo);
  tama = static_cast<int>(parte.size());
  return parte.data();
}

}  // namespace

multipoblacion::multipoblacion(int pbin, int pent, int preal, const int* rangoe, const double* mut,
                               int n, const int* tama, int n_val)
    : n_individuos_(n),
      n_valoracion_(n_val),
      modificado_(indice(n), true),
      valoracion_(indice(n), std::vector<double>(indice(n_val))) {
  for (int j = 0; j < pbin; ++j) {
    Pb_.emplace_back(mut[j], n, tama[j]);
  }
  for (int j = 0; j < pent; ++j) {
    Pe_.emplace_back(rangoe[j], mut[j + pbin], n, tama[j + pbin]);
  }
  for (int j = 0; j < preal; ++j) {
    Pr_.emplace_back(mut[j + pbin + pent], n, tama[j + pbin + pent]);
  }
}

void multipoblacion::Swap(int i, int j) {
  for (poblacionbinaria& p : Pb_) {
    p.Swap(i, j);
  }
  for (poblacionentera& p : Pe_) {
    p.Swap(i, j);
  }
  for (poblacionreal& p : Pr_) {
    p.Swap(i, j);
  }
  std::vector<bool>::swap(modificado_[indice(i)], modificado_[indice(j)]);
  std::swap(valoracion_[indice(i)], valoracion_[indice(j)]);
}

void multipoblacion::Pasar(int i, int j) {
  for (poblacionbinaria& p : Pb_) {
    p.Pasar(i, j);
  }
  for (poblacionentera& p : Pe_) {
    p.Pasar(i, j);
  }
  for (poblacionreal& p : Pr_) {
    p.Pasar(i, j);
  }
}

int multipoblacion::ClaseIndividuo(int ind) const { return Pe_[0].GetValue(ind, 0); }

char* multipoblacion::Subpoblacion_Binaria(int subpoblacion, int individuo, int& tama) {
  return Parte(Pb_, subpoblacion, individuo, tama);
}

int* multipoblacion::Subpoblacion_Entera(int subpoblacion, int individuo, int& tama) {
  return Parte(Pe_, subpoblacion, individuo, tama);
}

double* multipoblacion::Subpoblacion_Real(int subpoblacion, int individuo, int& tama) {
  return Parte(Pr_, subpoblacion, individuo, tama);
}

void multipoblacion::Sort() {
  for (int i = 0; i < n_individuos_ - 1; ++i) {
    for (int j = n_individuos_ - 1; j > i; --j) {
      if (Mayor(valoracion_[indice(j)], valoracion_[indice(j - 1)])) {
        Swap(j, j - 1);
      }
    }
  }
}

void multipoblacion::Sort_menos2ultimos() {
  bool cambio = true;
  for (int i = 0; cambio && i < n_individuos_ - 3; ++i) {
    cambio = false;
    for (int j = n_individuos_ - 3; j > i; --j) {
      if (Mayor(valoracion_[indice(j)], valoracion_[indice(j - 1)])) {
        Swap(j, j - 1);
        cambio = true;
      }
    }
  }
}

void multipoblacion::Sort_4L(int n_clases, int** ind_clase) {
  // Por inserción: el resto ya está ordenado.
  for (int i = n_individuos_ - 2; i < n_individuos_; ++i) {
    for (int k = i; k > 0 && Mayor(valoracion_[indice(k)], valoracion_[indice(k - 1)]); --k) {
      Swap(k, k - 1);
    }
  }

  for (int c = 0; c < n_clases; ++c) {
    ind_clase[c][0] = 0;
  }
  for (int i = 0; i < n_individuos_ - 2; ++i) {
    int* individuos = ind_clase[ClaseIndividuo(i)];
    ++individuos[0];
    individuos[individuos[0]] = i;
  }

  // Pocos individuos: los de la media por clase menos uno, o menos. (El original calculaba un
  // mínimo para cada clase, pero con tres casos que daban el mismo.)
  const int minimo = static_cast<int>((n_individuos_ - 2) / (1.0 * n_clases) - 1);
  bool usados_2_ultimos = true;
  for (int i = n_individuos_ - 2; i < n_individuos_; ++i) {
    int* individuos = ind_clase[ClaseIndividuo(i)];
    if (individuos[0] <= minimo) {
      usados_2_ultimos = false;
      int j = n_individuos_ - 3;
      while (ind_clase[ClaseIndividuo(j)][0] <= minimo) {
        --j;
      }
      int* otros = ind_clase[ClaseIndividuo(j)];
      individuos[individuos[0] + 1] = otros[otros[0]];
      ++individuos[0];
      --otros[0];
      Swap(i, j);
    }
  }
  if (!usados_2_ultimos) {
    Sort_menos2ultimos();
  }
}

void multipoblacion::Better(int* better, int& nbetter) {
  nbetter = 1;
  better[0] = 0;
}

void multipoblacion::PintaFitness_Estacionario(int i) const {
  std::ostream& salida = progreso(Verbosidad::depuracion);
  salida << "Fitnes: ";
  for (const double valor : valoracion_[indice(i)]) {
    salida << valor << "   ";
  }
  salida << "Clase => " << ClaseIndividuo(i) << '\n';
}

int multipoblacion::N_individuos() const { return n_individuos_; }

int multipoblacion::N_Valoracion() const { return n_valoracion_; }

void multipoblacion::Codigo(int i, genetcode& code) const {
  code.PutBinary(Partes(Pb_, i));
  code.PutInteger(Partes(Pe_, i));
  code.PutReal(Partes(Pr_, i));
}

void multipoblacion::PutCodigo(int i, const genetcode& code) {
  for (std::size_t j = 0; j < Pb_.size(); ++j) {
    const int fila = static_cast<int>(j);
    for (int k = 0; k < code.TamaBinary(fila); ++k) {
      Pb_[j].PutValue(i, k, code.GetValueBinary(fila, k));
    }
  }
  for (std::size_t j = 0; j < Pe_.size(); ++j) {
    const int fila = static_cast<int>(j);
    for (int k = 0; k < code.TamaInteger(fila); ++k) {
      Pe_[j].PutValue(i, k, code.GetValueInteger(fila, k));
    }
  }
  for (std::size_t j = 0; j < Pr_.size(); ++j) {
    const int fila = static_cast<int>(j);
    for (int k = 0; k < code.TamaReal(fila); ++k) {
      Pr_[j].PutValue(i, k, code.GetValueReal(fila, k));
    }
  }
}

bool multipoblacion::Modificado(int i) const { return modificado_[indice(i)]; }

void multipoblacion::PutModificado(int i) { modificado_[indice(i)] = true; }

void multipoblacion::Valoracion(int i, const double* dato) {
  std::vector<double>& valoracion = valoracion_[indice(i)];
  valoracion.assign(dato, dato + n_valoracion_);
  modificado_[indice(i)] = false;
}

double multipoblacion::ValorFitness(int i) const { return valoracion_[indice(i)][0]; }

double multipoblacion::ValorFitness(int i, int j) const {
  return valoracion_[indice(i)][indice(j)];
}

void multipoblacion::PoblacionInicial(const double* const* I, int rango, int n_items,
                                      const int* const* sujetos, int n_var,
                                      const int* n_etiquetas_variable) {
  Pb_[0].PoblacionInicial(sujetos, n_var, n_etiquetas_variable);
  Pb_[1].PoblacionInicialAleatoria();
  Pe_[0].PoblacionInicialAleatoria();
  Pe_[1].PoblacionInicialAleatoria(0);
  Pr_[0].PoblacionInicial(I, rango);
  Pr_[1].PoblacionInicialAleatoria(n_items);
  modificado_.assign(modificado_.size(), true);
  for (std::vector<double>& valoracion : valoracion_) {
    valoracion.assign(valoracion.size(), 0);
  }
}

void multipoblacion::MutacionUniforme_Estacionario_Especial() {
  if (Probabilidad(0.5)) {
    Pe_[1].MutacionUniformeMod_Estacionario_Especial();
  } else if (Probabilidad(0.5)) {
    Pr_[0].MutacionUniforme_Estacionario();
  }
}

void multipoblacion::MutacionUniforme_Estacionario() {
  bool mutado = false;
  while (!mutado) {
    if (Probabilidad(0.5)) {
      Pb_[0].MutacionUniforme_Estacionario();
      mutado = true;
    }
    if (Probabilidad(0.5)) {
      Pb_[1].MutacionUniforme_Estacionario();
      mutado = true;
    }
    if (Probabilidad(0.5)) {
      Pe_[0].MutacionUniforme_Estacionario();
      mutado = true;
    }
    if (Probabilidad(0.8)) {
      Pe_[1].MutacionUniformeMod_Estacionario(0.2);
      mutado = true;
    }
    if (Probabilidad(0.8)) {
      Pr_[0].MutacionUniforme_Estacionario();
      mutado = true;
    }
  }
}

void multipoblacion::CruceBasedLogical_Estacionario(double it) {
  const int penultimo = n_individuos_ - 2;
  const int ultimo = n_individuos_ - 1;
  int a = 0;
  if (Probabilidad(1 - it)) {
    a = Select_Random_Individuo(penultimo, -1, 0);
  }
  const int b = Select_Random_Individuo(penultimo, a, 0);
  Pasar(penultimo, a);
  Pasar(ultimo, b);

  // En el original, con probabilidad 1 este cruce y si no, otros (AND/OR y NAND/NOR) que nunca
  // se aplicaban. Se mantiene por el número que pide.
  static_cast<void>(Probabilidad(1.0));
  bool cruzado = false;
  while (!cruzado) {
    if (Probabilidad(0.25)) {
      Pb_[0].Cruce2puntos_Estacionario(penultimo, ultimo);
      cruzado = true;
    }
    if (Probabilidad(0.25)) {
      Pb_[1].Cruce2puntos_Estacionario(penultimo, ultimo);
      cruzado = true;
    }
    if (Probabilidad(0.25)) {
      Pe_[1].Cruce2puntos_Estacionario(penultimo, ultimo);
      cruzado = true;
    }
    if (Probabilidad(0.25)) {
      Pr_[0].Cruce2puntos_Estacionario(penultimo, ultimo);
      cruzado = true;
    }
  }
  modificado_[indice(penultimo)] = true;
  modificado_[indice(ultimo)] = true;
}

}  // namespace inslv
