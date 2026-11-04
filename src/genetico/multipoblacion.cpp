// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/multipoblacion.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "inslv/genetico/operaciones.h"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Si la valoración v1 es mejor que la v2: en el primer valor en el que no son iguales, el de v1
// es mayor.
bool Mayor(const Fitness& v1, const Fitness& v2) {
  std::size_t k = 0;
  while (k < v1.size() && v1[k] == v2[k]) {
    ++k;
  }
  return k < v1.size() && v1[k] > v2[k];
}

// Pide números al azar que no se usan: los de las poblaciones del original que no influían en
// nada (ver la clase).
void Gastar(int numeros) {
  for (int i = 0; i < numeros; ++i) {
    Aleatorio();
  }
}

// Los bits del nivel de valor en funciones y las posiciones que pide un cruce en dos puntos.
constexpr int bits_de_funciones = 40;
constexpr int puntos_de_cruce = 2;

}  // namespace

multipoblacion::multipoblacion(int clases, int n, int tama_valor, int tama_nivel, double mut_valor,
                               double mut_clase, double mut_nivel)
    : n_individuos_(n),
      n_clases_(clases),
      modificado_(indice(n), true),
      valoracion_(indice(n), Fitness{}),
      valor_(mut_valor, n, tama_valor),
      clase_(clases, mut_clase, n, 1),
      nivel_(mut_nivel, n, tama_nivel) {}

void multipoblacion::Swap(int i, int j) {
  valor_.Swap(i, j);
  clase_.Swap(i, j);
  nivel_.Swap(i, j);
  std::vector<bool>::swap(modificado_[indice(i)], modificado_[indice(j)]);
  std::swap(valoracion_[indice(i)], valoracion_[indice(j)]);
}

void multipoblacion::Pasar(int i, int j) {
  valor_.Pasar(i, j);
  clase_.Pasar(i, j);
  nivel_.Pasar(i, j);
}

int multipoblacion::ClaseIndividuo(int ind) const { return clase_.Individuo(ind)[0]; }

std::string& multipoblacion::Etiquetas(int individuo) { return valor_.Individuo(individuo); }

const std::string& multipoblacion::Etiquetas(int individuo) const {
  return valor_.Individuo(individuo);
}

std::vector<double>& multipoblacion::Niveles(int individuo) { return nivel_.Individuo(individuo); }

const std::vector<double>& multipoblacion::Niveles(int individuo) const {
  return nivel_.Individuo(individuo);
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

void multipoblacion::Sort_4L() {
  // Por inserción: el resto ya está ordenado.
  for (int i = n_individuos_ - 2; i < n_individuos_; ++i) {
    for (int k = i; k > 0 && Mayor(valoracion_[indice(k)], valoracion_[indice(k - 1)]); --k) {
      Swap(k, k - 1);
    }
  }

  // Cuántos individuos hay de cada clase, sin los dos últimos.
  std::vector<int> por_clase(indice(n_clases_));
  for (int i = 0; i < n_individuos_ - 2; ++i) {
    ++por_clase[indice(ClaseIndividuo(i))];
  }

  // Pocos individuos: los de la media por clase menos uno, o menos. (El original calculaba un
  // mínimo para cada clase, pero con tres casos que daban el mismo.)
  const int minimo = static_cast<int>((n_individuos_ - 2) / (1.0 * n_clases_) - 1);
  bool usados_2_ultimos = true;
  for (int i = n_individuos_ - 2; i < n_individuos_; ++i) {
    int& individuos = por_clase[indice(ClaseIndividuo(i))];
    if (individuos <= minimo) {
      usados_2_ultimos = false;
      int j = n_individuos_ - 3;
      while (por_clase[indice(ClaseIndividuo(j))] <= minimo) {
        --j;
      }
      ++individuos;
      --por_clase[indice(ClaseIndividuo(j))];
      Swap(i, j);
    }
  }
  if (!usados_2_ultimos) {
    Sort_menos2ultimos();
  }
}

int multipoblacion::N_individuos() const { return n_individuos_; }

genetcode multipoblacion::Codigo(int i) const {
  return {valor_.Individuo(i), ClaseIndividuo(i), nivel_.Individuo(i)};
}

void multipoblacion::PutCodigo(int i, const genetcode& code) {
  valor_.Individuo(i) = code.etiquetas;
  clase_.Individuo(i)[0] = code.clase;
  nivel_.Individuo(i) = code.niveles;
}

bool multipoblacion::Modificado(int i) const { return modificado_[indice(i)]; }

void multipoblacion::PutModificado(int i) { modificado_[indice(i)] = true; }

void multipoblacion::Valorar(int i, const Fitness& valoracion) {
  valoracion_[indice(i)] = valoracion;
  modificado_[indice(i)] = false;
}

const Fitness& multipoblacion::Valoracion(int i) const { return valoracion_[indice(i)]; }

void multipoblacion::PoblacionInicial(const std::vector<std::vector<double>>& I,
                                      const std::vector<std::vector<int>>& sujetos,
                                      const std::vector<int>& n_etiquetas_variable) {
  valor_.PoblacionInicial(sujetos, n_etiquetas_variable);
  Gastar(n_individuos_ * bits_de_funciones);
  clase_.PoblacionInicialAleatoria();
  nivel_.PoblacionInicial(I, n_clases_);
  modificado_.assign(modificado_.size(), true);
  valoracion_.assign(valoracion_.size(), Fitness{});
}

void multipoblacion::MutacionUniforme_Estacionario_Especial() {
  if (!Probabilidad(0.5) && Probabilidad(0.5)) {
    nivel_.MutacionUniforme_Estacionario();
  }
}

void multipoblacion::MutacionUniforme_Estacionario() {
  bool mutado = false;
  while (!mutado) {
    if (Probabilidad(0.5)) {
      valor_.MutacionUniforme_Estacionario();
      mutado = true;
    }
    if (Probabilidad(0.5)) {
      Gastar(2 * bits_de_funciones);
      mutado = true;
    }
    if (Probabilidad(0.5)) {
      clase_.MutacionUniforme_Estacionario();
      mutado = true;
    }
    if (Probabilidad(0.8)) {
      mutado = true;
    }
    if (Probabilidad(0.8)) {
      nivel_.MutacionUniforme_Estacionario();
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
      valor_.Cruce2puntos_Estacionario(penultimo, ultimo);
      cruzado = true;
    }
    if (Probabilidad(0.25)) {
      Gastar(puntos_de_cruce);
      cruzado = true;
    }
    if (Probabilidad(0.25)) {
      Gastar(puntos_de_cruce);
      cruzado = true;
    }
    if (Probabilidad(0.25)) {
      nivel_.Cruce2puntos_Estacionario(penultimo, ultimo);
      cruzado = true;
    }
  }
  modificado_[indice(penultimo)] = true;
  modificado_[indice(ultimo)] = true;
}

}  // namespace inslv
