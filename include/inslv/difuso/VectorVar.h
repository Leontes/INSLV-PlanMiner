// SPDX-License-Identifier: GPL-3.0-only
// Las variables de un problema: los atributos y el consecuente (la clase).
#pragma once

#include <string>
#include <vector>

#include "inslv/datos/ejemplo.hpp"
#include "inslv/difuso/domain_t.h"
#include "inslv/difuso/fuzzy_t.h"
#include "inslv/difuso/variable_t.h"
#include "inslv/inslv.hpp"

namespace inslv {

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
  /// Si la regla es válida: cada atributo relevante admite alguna etiqueta. etiquetas: las de cada
  /// atributo, seguidas, en 0 y 1; niveles: el de cada atributo y, al final, el umbral. Un atributo
  /// es relevante si su nivel llega al umbral.
  [[nodiscard]] bool Es_Valida(const std::string& etiquetas,
                               const std::vector<double>& niveles) const;
  /// La adaptación del ejemplo a la regla: la menor de los atributos relevantes que no admiten
  /// todas sus etiquetas (0 si alguno no admite ninguna).
  [[nodiscard]] double Adaptacion(const datos::Ejemplo& x, const std::string& etiquetas,
                                  const std::vector<double>& niveles) const;
  [[nodiscard]] const domain_t& Domain(int var) const;
  [[nodiscard]] const variable_t& Variable(int var) const;
  /// La posición del consecuente, o -1 si no hay.
  [[nodiscard]] int Consecuente() const;

 private:
  std::vector<variable_t> lista_;
};

}  // namespace inslv
