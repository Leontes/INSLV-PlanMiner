// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          genetcode.cpp  -  description
                             -------------------
    begin                : Tue Apr 30 2002
    copyright            : (C) 2002 by Raúl Pérez
    email                : fgr@decsai.ugr.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "inslv/genetico/genetcode.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

template <typename T>
void genetcode::Matriz<T>::Asignar(std::vector<std::vector<T>> nuevas) {
  filas = std::move(nuevas);
  tamanos.clear();
  for (const std::vector<T>& fila : filas) {
    tamanos.push_back(static_cast<int>(fila.size()));
  }
  RehacerPunteros();
}

template <typename T>
void genetcode::Matriz<T>::RehacerPunteros() {
  punteros.clear();
  for (std::vector<T>& fila : filas) {
    punteros.push_back(fila.data());
  }
}

genetcode::genetcode(const genetcode& x)
    : binario_(x.binario_), entero_(x.entero_), real_(x.real_) {
  binario_.RehacerPunteros();
  entero_.RehacerPunteros();
  real_.RehacerPunteros();
}

genetcode& genetcode::operator=(const genetcode& x) {
  if (this != &x) {
    genetcode copia(x);
    *this = std::move(copia);
  }
  return *this;
}

void genetcode::PutBinary(std::vector<std::vector<char>> filas) {
  binario_.Asignar(std::move(filas));
}

void genetcode::PutInteger(std::vector<std::vector<int>> filas) {
  entero_.Asignar(std::move(filas));
}

void genetcode::PutReal(std::vector<std::vector<double>> filas) { real_.Asignar(std::move(filas)); }

void genetcode::PutValueBinary(int fila, int columna, char value) {
  binario_.filas[indice(fila)][indice(columna)] = value;
}

void genetcode::PutValueInteger(int fila, int columna, int value) {
  entero_.filas[indice(fila)][indice(columna)] = value;
}

int genetcode::TamaBinary(int fila) const { return binario_.tamanos[indice(fila)]; }

int genetcode::TamaInteger(int fila) const { return entero_.tamanos[indice(fila)]; }

int genetcode::TamaReal(int fila) const { return real_.tamanos[indice(fila)]; }

void genetcode::GetBinary(int& bin, int*& nbin, char**& mbin) {
  bin = static_cast<int>(binario_.filas.size());
  nbin = binario_.tamanos.data();
  mbin = binario_.punteros.data();
}

void genetcode::GetInteger(int& ent, int*& nent, int**& ment) {
  ent = static_cast<int>(entero_.filas.size());
  nent = entero_.tamanos.data();
  ment = entero_.punteros.data();
}

void genetcode::GetReal(int& rea, int*& nrea, double**& mrea) {
  rea = static_cast<int>(real_.filas.size());
  nrea = real_.tamanos.data();
  mrea = real_.punteros.data();
}

char genetcode::GetValueBinary(int fila, int columna) const {
  return binario_.filas[indice(fila)][indice(columna)];
}

int genetcode::GetValueInteger(int fila, int columna) const {
  return entero_.filas[indice(fila)][indice(columna)];
}

double genetcode::GetValueReal(int fila, int columna) const {
  return real_.filas[indice(fila)][indice(columna)];
}

}  // namespace inslv
