// SPDX-License-Identifier: GPL-3.0-only
// Las variables de un problema: los atributos y el consecuente (la clase).
#pragma once

#include <array>
#include <string>
#include <vector>

#include "inslv/datos/vectordouble.h"
#include "inslv/difuso/domain_t.h"
#include "inslv/difuso/fuzzy_t.h"
#include "inslv/difuso/variable_t.h"
#include "inslv/inslv.hpp"

namespace inslv {

/// Una variable construida a partir de otras dos (operando[0] op operando[1]). El original las
/// usaba para descubrir relaciones entre atributos; nunca se crean, así que op es siempre 0.
struct combinado {
  int op = 0;
  std::array<int, 2> operando{};
};

class VectorVar {
 public:
  VectorVar() = default;
  /// Los atributos de los datos y, al final, la clase como consecuente.
  explicit VectorVar(const Datos& data);

  /// Las variables activas que son atributos.
  [[nodiscard]] int N_Antecedente() const;
  /// tb: el total de etiquetas de los atributos; te: las del consecuente.
  void Encode(int& tb, int& te) const;
  /// Las variables, a la depuración.
  void Pinta() const;
  void PrintVar(int variable) const;
  [[nodiscard]] std::string SPrintVar(int variable) const;
  [[nodiscard]] std::string SPrintDomain(int variable, int value) const;
  [[nodiscard]] bool IsActiva(int variable) const;
  [[nodiscard]] bool IsAntecedente(int variable) const;
  [[nodiscard]] int TotalVariables() const;
  [[nodiscard]] int SizeDomain(int variable) const;
  /// La adaptación del valor a una etiqueta de la variable. Lanza std::logic_error si no existe.
  [[nodiscard]] double Adaptacion(double x, int variable, int dominio) const;
  /// Si la regla (las etiquetas de cada atributo, en 0 y 1) es válida: cada atributo relevante
  /// (var[i] >= umbral) admite alguna etiqueta. simplicidad: los que admiten una sola secuencia de
  /// etiquetas seguidas o excluyen una sola.
  [[nodiscard]] bool Es_Valida(const std::string& regla, const double* var, double umbral,
                               double& simplicidad) const;
  /// La adaptación del ejemplo a la regla: la menor de los atributos relevantes que no admiten
  /// todas sus etiquetas (0 si alguno no admite ninguna).
  [[nodiscard]] double Adaptacion(const vectordouble& x, const std::string& regla,
                                  const double* var, double umbral) const;
  /// La adaptación a la parte de la regla de las variables combinadas, que nunca hay: devuelve
  /// adaptacion_previa si lista_funcion no tiene ninguna.
  [[nodiscard]] double Adaptacion_funcion(const vectordouble& x, const std::string& regla,
                                          const int* lista_funcion, int tama,
                                          double adaptacion_previa) const;
  [[nodiscard]] const domain_t& Domain(int var) const;
  [[nodiscard]] const variable_t& Variable(int var) const;
  /// La posición del consecuente, o -1 si no hay.
  [[nodiscard]] int Consecuente() const;
  void Funcion(int pos, int& x1, int& x2, int& op) const;

 private:
  std::vector<variable_t> lista_;
  std::vector<combinado> oper_;
};

}  // namespace inslv
