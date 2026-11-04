// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/VectorVar.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Cuántas etiquetas admite (los 1 de la cadena).
int NumeroEtiquetasActivas(const std::string& sub) {
  int n = 0;
  for (const char etiqueta : sub) {
    if (etiqueta == '1') {
      ++n;
    }
  }
  return n;
}

}  // namespace

VectorVar::VectorVar(const Datos& data) {
  const std::size_t numero = data.atributos.size() + 1;
  lista_.reserve(numero);
  for (std::size_t v = 0; v < numero; ++v) {
    const bool es_clase = v + 1 == numero;
    const inslv::Variable& variable = es_clase ? data.clase : data.atributos[v];
    std::vector<fuzzy_t> etiquetas;
    etiquetas.reserve(variable.etiquetas.size());
    for (const ConjuntoDifuso& etiqueta : variable.etiquetas) {
      etiquetas.emplace_back(etiqueta.a, etiqueta.b, etiqueta.c, etiqueta.d, etiqueta.etiqueta);
    }
    lista_.emplace_back(variable.nombre, es_clase ? 1 : 0, std::move(etiquetas));
  }
}

int VectorVar::N_Antecedente() const {
  int s = 0;
  for (const variable_t& variable : lista_) {
    if (variable.Activa() && variable.Antecedente()) {
      ++s;
    }
  }
  return s;
}

void VectorVar::Encode(int& tb, int& te) const {
  tb = 0;
  for (const variable_t& variable : lista_) {
    if (variable.Activa() && variable.Antecedente()) {
      tb += variable.N_etiquetas();
    } else if (variable.Activa() && !variable.Antecedente()) {
      te = variable.N_etiquetas();
    }
  }
}

void VectorVar::Pinta() const {
  for (const variable_t& variable : lista_) {
    comun::progreso(Verbosidad::depuracion) << '\n';
    variable.Pinta();
    comun::progreso(Verbosidad::depuracion) << "----------\n";
  }
}

void VectorVar::PrintVar(int variable) const { lista_[indice(variable)].PrintVar(); }

std::string VectorVar::SPrintVar(int variable) const {
  return lista_[indice(variable)].SPrintVar();
}

std::string VectorVar::SPrintDomain(int variable, int value) const {
  return lista_[indice(variable)].SPrintDomain(value);
}

bool VectorVar::IsActiva(int variable) const { return lista_[indice(variable)].Activa(); }

bool VectorVar::IsAntecedente(int variable) const { return lista_[indice(variable)].Antecedente(); }

int VectorVar::TotalVariables() const { return static_cast<int>(lista_.size()); }

int VectorVar::SizeDomain(int variable) const { return lista_[indice(variable)].SizeDomain(); }

double VectorVar::Adaptacion(double x, int variable, int dominio) const {
  if (variable < 0 || indice(variable) >= lista_.size()) {
    throw std::logic_error("inslv: no existe la variable " + std::to_string(variable));
  }
  return lista_[indice(variable)].Adaptacion(x, dominio);
}

bool VectorVar::Es_Valida(const std::string& etiquetas, const std::vector<double>& niveles) const {
  std::size_t trozo = 0;
  for (std::size_t i = 0; i < lista_.size(); ++i) {
    const variable_t& variable = lista_[i];
    if (!variable.Activa() || !variable.Antecedente()) {
      continue;
    }
    const auto tam = indice(variable.N_etiquetas());
    if (niveles[i] >= niveles.back() && NumeroEtiquetasActivas(etiquetas.substr(trozo, tam)) == 0) {
      return false;
    }
    trozo += tam;
  }
  return true;
}

double VectorVar::Adaptacion(const datos::Ejemplo& x, const std::string& etiquetas,
                             const std::vector<double>& niveles) const {
  double max = 1;
  std::size_t trozo = 0;
  for (std::size_t i = 0; i < lista_.size() && max > 0; ++i) {
    const variable_t& variable = lista_[i];
    if (!variable.Activa() || !variable.Antecedente()) {
      continue;
    }
    const int tam = variable.N_etiquetas();
    if (niveles[i] >= niveles.back()) {
      const std::string sub = etiquetas.substr(trozo, indice(tam));
      const int unos = NumeroEtiquetasActivas(sub);
      if (unos == 0) {
        max = 0;
      } else if (unos < tam) {
        const double aux = variable.Adaptacion(x.valor(static_cast<int>(i)), sub);
        if (aux < max) {
          max = aux;
        }
      }
    }
    trozo += indice(tam);
  }
  return max;
}

const domain_t& VectorVar::Domain(int var) const { return lista_[indice(var)].Domain(); }

const variable_t& VectorVar::Variable(int var) const { return lista_[indice(var)]; }

int VectorVar::Consecuente() const {
  for (std::size_t i = 0; i < lista_.size(); ++i) {
    if (lista_[i].Activa() && !lista_[i].Antecedente()) {
      return static_cast<int>(i);
    }
  }
  comun::progreso(Verbosidad::depuracion) << "No hay variable consecuente\n";
  return -1;
}

}  // namespace inslv
