// SPDX-License-Identifier: GPL-3.0-only
// Los valores de un ejemplo.
#pragma once

#include <vector>

namespace inslv::datos {

class Ejemplo {
 public:
  explicit Ejemplo(std::vector<double> valores);

  /// El valor en pos o, si no existe, el de un desconocido (-999999999).
  [[nodiscard]] double valor(int pos) const;

 private:
  std::vector<double> valores_;
};

}  // namespace inslv::datos
