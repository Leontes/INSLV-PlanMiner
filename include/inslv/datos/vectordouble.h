// SPDX-License-Identifier: GPL-3.0-only
// Los valores de un ejemplo: una fila de example_set.
#pragma once

#include <vector>

namespace inslv {

class vectordouble {
 public:
  vectordouble() = default;
  /// Vacío: tamano es solo la capacidad que se reserva.
  explicit vectordouble(int tamano);
  explicit vectordouble(std::vector<double> valores);

  /// El valor en pos o, si no existe, el de un desconocido (-999999999).
  [[nodiscard]] double At(int pos) const;

 private:
  std::vector<double> valores_;
};

}  // namespace inslv
