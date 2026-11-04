// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/variable_difusa.hpp"

#include <string>
#include <utility>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv::difuso {

VariableDifusa::VariableDifusa(std::string nombre, std::vector<Etiqueta> etiquetas)
    : nombre_(std::move(nombre)), dominio_(std::move(etiquetas)) {}

const std::string& VariableDifusa::nombre() const { return nombre_; }

const Dominio& VariableDifusa::dominio() const { return dominio_; }

void VariableDifusa::pintar() const {
  comun::progreso(Verbosidad::depuracion) << "Variable: " << nombre_ << '\n'
                                          << "========================\n";
  dominio_.pintar();
}

}  // namespace inslv::difuso
