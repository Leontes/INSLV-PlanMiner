// SPDX-License-Identifier: GPL-3.0-only
// Una variable: su nombre, su papel (atributo o consecuente) y su dominio.
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "inslv/difuso/domain_t.h"
#include "inslv/difuso/fuzzy_t.h"

namespace inslv {

class variable_t {
 public:
  variable_t() = default;
  /// estado: -1, inactiva; 0, atributo (antecedente); cualquier otro, el consecuente.
  variable_t(std::string nombre, int estado, std::vector<fuzzy_t> etiquetas);

  /// La adaptación del valor a una etiqueta o a un conjunto de etiquetas (domain_t). Lanzan
  /// std::logic_error si la variable no tiene dominio.
  [[nodiscard]] double Adaptacion(double x, int etiqueta) const;
  [[nodiscard]] double Adaptacion(double x, const std::string& etiquetas) const;
  /// La variable con su dominio, a la depuración.
  void Pinta() const;
  /// El nombre, a la depuración.
  void PrintVar() const;
  [[nodiscard]] std::string SPrintVar() const;
  [[nodiscard]] std::string SPrintDomain(int value) const;
  [[nodiscard]] int SizeDomain() const;
  [[nodiscard]] bool Activa() const;
  [[nodiscard]] bool Antecedente() const;
  [[nodiscard]] int N_etiquetas() const;
  [[nodiscard]] std::string Name() const;
  /// El dominio. Lanza std::logic_error si no tiene.
  [[nodiscard]] const domain_t& Domain() const;

 private:
  std::string nombre_ = "Sin asignar";
  std::optional<domain_t> dominio_;
  bool activa_ = false;
  bool antecedente_ = true;
};

}  // namespace inslv
