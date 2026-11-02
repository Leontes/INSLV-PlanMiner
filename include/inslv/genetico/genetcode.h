// SPDX-License-Identifier: GPL-3.0-only
// El código genético de un individuo (o de una regla): una matriz de bits, una de enteros y una
// de reales, cada una con sus filas de distinto tamaño. Parte de NSLV, de Raúl Pérez (2002).
#pragma once

#include <vector>

namespace inslv {

class genetcode {
 public:
  genetcode() = default;
  genetcode(const genetcode& x);
  genetcode& operator=(const genetcode& x);
  genetcode(genetcode&&) noexcept = default;
  genetcode& operator=(genetcode&&) noexcept = default;
  ~genetcode() = default;

  /// Cambia las filas.
  void PutBinary(std::vector<std::vector<char>> filas);
  void PutInteger(std::vector<std::vector<int>> filas);
  void PutReal(std::vector<std::vector<double>> filas);
  void PutValueBinary(int fila, int columna, char value);
  void PutValueInteger(int fila, int columna, int value);
  [[nodiscard]] int TamaBinary(int fila) const;
  [[nodiscard]] int TamaInteger(int fila) const;
  [[nodiscard]] int TamaReal(int fila) const;
  /// El número de filas, el tamaño de cada una y las filas. Los punteros son de este objeto: se
  /// puede escribir en las filas (sin cambiar su tamaño) y dejan de valer al cambiarlo.
  void GetBinary(int& bin, int*& nbin, char**& mbin);
  void GetInteger(int& ent, int*& nent, int**& ment);
  void GetReal(int& rea, int*& nrea, double**& mrea);
  [[nodiscard]] char GetValueBinary(int fila, int columna) const;
  [[nodiscard]] int GetValueInteger(int fila, int columna) const;
  [[nodiscard]] double GetValueReal(int fila, int columna) const;

 private:
  // Filas de distinto tamaño y, para GetBinary y compañía, sus tamaños y punteros a ellas.
  template <typename T>
  struct Matriz {
    std::vector<std::vector<T>> filas;
    std::vector<int> tamanos;
    std::vector<T*> punteros;

    void Asignar(std::vector<std::vector<T>> nuevas);
    void RehacerPunteros();
  };

  Matriz<char> binario_;
  Matriz<int> entero_;
  Matriz<double> real_;
};

}  // namespace inslv
