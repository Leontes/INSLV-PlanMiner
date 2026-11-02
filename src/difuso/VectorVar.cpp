// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/difuso/VectorVar.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

constexpr double desconocido = -999999999;

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

// Las secuencias de 1 y de 0 seguidos de la cadena.
void SecuenciasDeEtiquetasActivas(const std::string& sub, int& unos, int& ceros) {
  unos = 0;
  ceros = 0;
  bool anterior_uno = sub[0] == '1';
  if (anterior_uno) {
    ++unos;
  } else {
    ++ceros;
  }
  for (std::size_t i = 1; i < sub.size(); ++i) {
    if (anterior_uno && sub[i] == '0') {
      anterior_uno = false;
      ++ceros;
    } else if (!anterior_uno && sub[i] == '1') {
      anterior_uno = true;
      ++unos;
    }
  }
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
  oper_.resize(numero);
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
    progreso(Verbosidad::depuracion) << '\n';
    variable.Pinta();
    progreso(Verbosidad::depuracion) << "----------\n";
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

bool VectorVar::Es_Valida(const std::string& regla, const double* var, double umbral,
                          double& simplicidad) const {
  simplicidad = 0;
  std::size_t trozo = 0;
  for (std::size_t i = 0; i < lista_.size(); ++i) {
    const variable_t& variable = lista_[i];
    if (!variable.Activa() || !variable.Antecedente()) {
      continue;
    }
    const auto tam = indice(variable.N_etiquetas());
    if (var[i] >= umbral) {
      int unos = 0;
      int ceros = 0;
      SecuenciasDeEtiquetasActivas(regla.substr(trozo, tam), unos, ceros);
      if (unos == 0) {
        return false;
      }
      if (unos == 1 || ceros == 1) {
        simplicidad = simplicidad + 1;
      }
    }
    trozo += tam;
  }
  return true;
}

double VectorVar::Adaptacion(const vectordouble& x, const std::string& regla, const double* var,
                             double umbral) const {
  double max = 1;
  std::size_t trozo = 0;
  for (std::size_t i = 0; i < lista_.size() && max > 0; ++i) {
    const variable_t& variable = lista_[i];
    if (!variable.Activa() || !variable.Antecedente()) {
      continue;
    }
    const int tam = variable.N_etiquetas();
    if (var[i] >= umbral) {
      const std::string sub = regla.substr(trozo, indice(tam));
      const int unos = NumeroEtiquetasActivas(sub);
      if (unos == 0) {
        max = 0;
      } else if (unos < tam) {
        const double aux = variable.Adaptacion(x.At(static_cast<int>(i)), sub);
        if (aux < max) {
          max = aux;
        }
      }
    }
    trozo += indice(tam);
  }
  return max;
}

double VectorVar::Adaptacion_funcion(const vectordouble& x, const std::string& regla,
                                     const int* lista_funcion, int tama,
                                     double adaptacion_previa) const {
  double max = adaptacion_previa;
  std::size_t trozo = 0;
  for (int i = 0; i < tama && max > 0; ++i) {
    if (lista_funcion[i] != 0) {
      const std::size_t f = indice(lista_funcion[i] - 1);
      const std::string sub = regla.substr(trozo, indice(lista_[f].N_etiquetas()));
      const double x1 = x.At(oper_[f].operando[0]);
      const double x2 = x.At(oper_[f].operando[1]);
      double resultado = 0;
      if (x1 == desconocido || x2 == desconocido) {
        resultado = desconocido;
      } else {
        switch (oper_[f].op) {
          case 1:
            resultado = x1 >= x2 ? 1 : 0;
            break;
          case 2:
            resultado = x1 + x2;
            break;
          case 3:
            resultado = x1 * x2;
            break;
          case 4:
            resultado = x1 - x2;
            break;
          case 5:
            if (x2 == 0) {
              return desconocido;
            }
            resultado = x1 / x2;
            break;
          case 19:
            resultado = x1 == x2 ? 1 : 0;
            break;
          case 7:
          case 8:
            resultado = x1;
            break;
          default:
            max = 0;
            break;
        }
      }
      // En el original, aux quedaba sin inicializar si max ya era 0.
      const double aux = max > 0 ? lista_[f].Adaptacion(resultado, sub) : max;
      if (aux < max) {
        max = aux;
      }
    }
    trozo += 2;
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
  progreso(Verbosidad::depuracion) << "No hay variable consecuente\n";
  return -1;
}

void VectorVar::Funcion(int pos, int& x1, int& x2, int& op) const {
  const combinado& funcion = oper_[indice(pos)];
  x1 = funcion.operando[0];
  x2 = funcion.operando[1];
  op = funcion.op;
}

}  // namespace inslv
