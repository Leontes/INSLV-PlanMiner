// SPDX-License-Identifier: GPL-3.0-only
// Las medidas de información entre los atributos y la clase.
#pragma once

#include <vector>

#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"

namespace inslv {

/// Una matriz con una fila por atributo y una columna por clase más una: la información mutua
/// normalizada entre el atributo y cada clase y, en la columna 0, con todas.
[[nodiscard]] std::vector<std::vector<double>> MedidasDeInformacion(const difuso::Variables& V,
                                                                    const datos::Ejemplos& E);

}  // namespace inslv
