// SPDX-License-Identifier: GPL-3.0-only
// Las variables de un problema: los atributos y, al final, la clase.
#pragma once

#include <string>
#include <vector>

#include "inslv/datos/ejemplo.hpp"
#include "inslv/difuso/variable_difusa.hpp"
#include "inslv/inslv.hpp"

namespace inslv::difuso {

class Variables {
 public:
  Variables() = default;
  /// Los atributos de los datos y, al final, la clase.
  explicit Variables(const Datos& data);

  /// Los atributos y la clase.
  [[nodiscard]] int numero_de_variables() const;
  [[nodiscard]] int numero_de_atributos() const;
  /// La posición de la clase, o -1 si no hay variables.
  [[nodiscard]] int consecuente() const;
  [[nodiscard]] int numero_de_etiquetas(int variable) const;
  /// El total de etiquetas de los atributos.
  [[nodiscard]] int etiquetas_de_atributos() const;
  [[nodiscard]] const std::string& nombre(int variable) const;
  [[nodiscard]] const std::string& nombre_de_etiqueta(int variable, int etiqueta) const;
  /// Las variables, a la depuración.
  void pintar() const;
  /// El nombre de la variable, a la depuración.
  void pintar_nombre(int variable) const;

  /// La adaptación del valor a una etiqueta de la variable. Lanza std::logic_error si no existe.
  [[nodiscard]] double adaptacion(double x, int variable, int etiqueta) const;
  /// La adaptación del ejemplo a la regla: la menor de los atributos relevantes que no admiten
  /// todas sus etiquetas (0 si alguno no admite ninguna). etiquetas: las de cada atributo,
  /// seguidas, en 0 y 1; niveles: el de cada atributo y, al final, el umbral. Un atributo es
  /// relevante si su nivel llega al umbral.
  [[nodiscard]] double adaptacion(const datos::Ejemplo& x, const std::string& etiquetas,
                                  const std::vector<double>& niveles) const;
  /// Si la regla es válida: cada atributo relevante admite alguna etiqueta.
  [[nodiscard]] bool es_valida(const std::string& etiquetas,
                               const std::vector<double>& niveles) const;

 private:
  std::vector<VariableDifusa> variables_;
};

}  // namespace inslv::difuso
