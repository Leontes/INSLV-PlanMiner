// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/genetico/multipoblacion.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "inslv/genetico/azar.hpp"

namespace inslv::genetico {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Si la valoración v1 es mejor que la v2: en el primer valor en el que no son iguales, el de v1
// es mayor.
bool mayor(const Valoracion& v1, const Valoracion& v2) {
  std::size_t k = 0;
  while (k < v1.size() && v1[k] == v2[k]) {
    ++k;
  }
  return k < v1.size() && v1[k] > v2[k];
}

// Pide números al azar que no se usan: los de las poblaciones del original que no influían en
// nada (ver la clase).
void gastar(int numeros) {
  for (int i = 0; i < numeros; ++i) {
    aleatorio();
  }
}

// Los bits del nivel de valor en funciones y las posiciones que pide un cruce en dos puntos.
constexpr int bits_de_funciones = 40;
constexpr int puntos_de_cruce = 2;

}  // namespace

MultiPoblacion::MultiPoblacion(int clases, int n, int tama_valor, int tama_nivel, double mut_valor,
                               double mut_clase, double mut_nivel)
    : n_individuos_(n),
      n_clases_(clases),
      modificado_(indice(n), true),
      valoracion_(indice(n), Valoracion{}),
      etiquetas_(mut_valor, n, tama_valor),
      clases_(clases, mut_clase, n, 1),
      niveles_(mut_nivel, n, tama_nivel) {}

void MultiPoblacion::intercambiar(int i, int j) {
  etiquetas_.intercambiar(i, j);
  clases_.intercambiar(i, j);
  niveles_.intercambiar(i, j);
  std::vector<bool>::swap(modificado_[indice(i)], modificado_[indice(j)]);
  std::swap(valoracion_[indice(i)], valoracion_[indice(j)]);
}

void MultiPoblacion::copiar(int i, int j) {
  etiquetas_.copiar(i, j);
  clases_.copiar(i, j);
  niveles_.copiar(i, j);
}

int MultiPoblacion::clase(int ind) const { return clases_.individuo(ind)[0]; }

std::string& MultiPoblacion::etiquetas(int individuo) { return etiquetas_.individuo(individuo); }

const std::string& MultiPoblacion::etiquetas(int individuo) const {
  return etiquetas_.individuo(individuo);
}

std::vector<double>& MultiPoblacion::niveles(int individuo) {
  return niveles_.individuo(individuo);
}

const std::vector<double>& MultiPoblacion::niveles(int individuo) const {
  return niveles_.individuo(individuo);
}

void MultiPoblacion::ordenar() {
  for (int i = 0; i < n_individuos_ - 1; ++i) {
    for (int j = n_individuos_ - 1; j > i; --j) {
      if (mayor(valoracion_[indice(j)], valoracion_[indice(j - 1)])) {
        intercambiar(j, j - 1);
      }
    }
  }
}

void MultiPoblacion::ordenar_sin_los_dos_ultimos() {
  bool cambio = true;
  for (int i = 0; cambio && i < n_individuos_ - 3; ++i) {
    cambio = false;
    for (int j = n_individuos_ - 3; j > i; --j) {
      if (mayor(valoracion_[indice(j)], valoracion_[indice(j - 1)])) {
        intercambiar(j, j - 1);
        cambio = true;
      }
    }
  }
}

void MultiPoblacion::colocar_los_dos_ultimos() {
  // Por inserción: el resto ya está ordenado.
  for (int i = n_individuos_ - 2; i < n_individuos_; ++i) {
    for (int k = i; k > 0 && mayor(valoracion_[indice(k)], valoracion_[indice(k - 1)]); --k) {
      intercambiar(k, k - 1);
    }
  }

  // Cuántos individuos hay de cada clase, sin los dos últimos.
  std::vector<int> por_clase(indice(n_clases_));
  for (int i = 0; i < n_individuos_ - 2; ++i) {
    ++por_clase[indice(clase(i))];
  }

  // Pocos individuos: los de la media por clase menos uno, o menos. (El original calculaba un
  // mínimo para cada clase, pero con tres casos que daban el mismo.)
  const int minimo = static_cast<int>((n_individuos_ - 2) / (1.0 * n_clases_) - 1);
  bool usados_2_ultimos = true;
  for (int i = n_individuos_ - 2; i < n_individuos_; ++i) {
    int& individuos = por_clase[indice(clase(i))];
    if (individuos <= minimo) {
      usados_2_ultimos = false;
      int j = n_individuos_ - 3;
      while (por_clase[indice(clase(j))] <= minimo) {
        --j;
      }
      ++individuos;
      --por_clase[indice(clase(j))];
      intercambiar(i, j);
    }
  }
  if (!usados_2_ultimos) {
    ordenar_sin_los_dos_ultimos();
  }
}

int MultiPoblacion::numero_de_individuos() const { return n_individuos_; }

Codigo MultiPoblacion::codigo(int i) const {
  return {etiquetas_.individuo(i), clase(i), niveles_.individuo(i)};
}

void MultiPoblacion::poner_codigo(int i, const Codigo& code) {
  etiquetas_.individuo(i) = code.etiquetas;
  clases_.individuo(i)[0] = code.clase;
  niveles_.individuo(i) = code.niveles;
}

bool MultiPoblacion::modificado(int i) const { return modificado_[indice(i)]; }

void MultiPoblacion::marcar_modificado(int i) { modificado_[indice(i)] = true; }

void MultiPoblacion::valorar(int i, const Valoracion& valoracion) {
  valoracion_[indice(i)] = valoracion;
  modificado_[indice(i)] = false;
}

const Valoracion& MultiPoblacion::valoracion(int i) const { return valoracion_[indice(i)]; }

void MultiPoblacion::inicializar(const std::vector<std::vector<double>>& I,
                                 const std::vector<std::vector<int>>& sujetos,
                                 const std::vector<int>& n_etiquetas_variable) {
  etiquetas_.inicializar(sujetos, n_etiquetas_variable);
  gastar(n_individuos_ * bits_de_funciones);
  clases_.inicializar();
  niveles_.inicializar(I, n_clases_);
  modificado_.assign(modificado_.size(), true);
  valoracion_.assign(valoracion_.size(), Valoracion{});
}

void MultiPoblacion::mutar_niveles() {
  if (!con_probabilidad(0.5) && con_probabilidad(0.5)) {
    niveles_.mutar();
  }
}

void MultiPoblacion::mutar() {
  bool mutado = false;
  while (!mutado) {
    if (con_probabilidad(0.5)) {
      etiquetas_.mutar();
      mutado = true;
    }
    if (con_probabilidad(0.5)) {
      gastar(2 * bits_de_funciones);
      mutado = true;
    }
    if (con_probabilidad(0.5)) {
      clases_.mutar();
      mutado = true;
    }
    if (con_probabilidad(0.8)) {
      mutado = true;
    }
    if (con_probabilidad(0.8)) {
      niveles_.mutar();
      mutado = true;
    }
  }
}

void MultiPoblacion::cruzar(double it) {
  const int penultimo = n_individuos_ - 2;
  const int ultimo = n_individuos_ - 1;
  int a = 0;
  if (con_probabilidad(1 - it)) {
    a = individuo_al_azar(penultimo, -1, 0);
  }
  const int b = individuo_al_azar(penultimo, a, 0);
  copiar(penultimo, a);
  copiar(ultimo, b);

  // En el original, con probabilidad 1 este cruce y si no, otros (AND/OR y NAND/NOR) que nunca
  // se aplicaban. Se mantiene por el número que pide.
  static_cast<void>(con_probabilidad(1.0));
  bool cruzado = false;
  while (!cruzado) {
    if (con_probabilidad(0.25)) {
      etiquetas_.cruzar_en_dos_puntos(penultimo, ultimo);
      cruzado = true;
    }
    if (con_probabilidad(0.25)) {
      gastar(puntos_de_cruce);
      cruzado = true;
    }
    if (con_probabilidad(0.25)) {
      gastar(puntos_de_cruce);
      cruzado = true;
    }
    if (con_probabilidad(0.25)) {
      niveles_.cruzar_en_dos_puntos(penultimo, ultimo);
      cruzado = true;
    }
  }
  modificado_[indice(penultimo)] = true;
  modificado_[indice(ultimo)] = true;
}

}  // namespace inslv::genetico
