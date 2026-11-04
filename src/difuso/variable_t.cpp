// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/variable_t.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv {

variable_t::variable_t(std::string nombre, int estado, std::vector<fuzzy_t> etiquetas)
    : nombre_(std::move(nombre)),
      dominio_(domain_t(std::move(etiquetas))),
      activa_(estado != -1),
      antecedente_(estado == -1 || estado == 0) {}

const domain_t& variable_t::Domain() const {
  if (!dominio_) {
    throw std::logic_error("inslv: la variable " + nombre_ + " no tiene dominio");
  }
  return *dominio_;
}

double variable_t::Adaptacion(double x, int etiqueta) const {
  return Domain().Adaptacion(x, etiqueta);
}

double variable_t::Adaptacion(double x, const std::string& etiquetas) const {
  return Domain().Adaptacion(x, etiquetas);
}

void variable_t::Pinta() const {
  const domain_t& dominio = Domain();
  comun::progreso(Verbosidad::depuracion) << "Variable: " << nombre_ << '\n'
                                          << "========================\n";
  dominio.Pinta();
}

void variable_t::PrintVar() const { comun::progreso(Verbosidad::depuracion) << nombre_; }

std::string variable_t::SPrintVar() const { return nombre_; }

std::string variable_t::SPrintDomain(int value) const { return Domain().SPrint(value); }

int variable_t::SizeDomain() const { return Domain().N_etiquetas(); }

bool variable_t::Activa() const { return activa_; }

bool variable_t::Antecedente() const { return antecedente_; }

int variable_t::N_etiquetas() const { return Domain().N_etiquetas(); }

std::string variable_t::Name() const { return nombre_; }

}  // namespace inslv
