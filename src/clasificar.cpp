// SPDX-License-Identifier: GPL-3.0-only
// La clasificación con las reglas: la inferencia de NSLV (reglas::ConjuntoDeReglas::inferir en
// old/), con los mismos cálculos para dar las mismas clases bit a bit.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "inslv/inslv.hpp"

namespace inslv {

namespace {

// El valor que marca un dato desconocido en la implementación (como NaN en la interfaz).
constexpr double desconocido = -999999999;

// La adaptación de un valor a una etiqueta, como fuzzy_t::Adaptacion.
double adaptacion(double valor, const ConjuntoDifuso& etiqueta) {
  if (std::isnan(valor) || valor == desconocido) {
    return 1;
  }
  if (valor < etiqueta.a) {
    return 0;
  }
  if (valor < etiqueta.b) {
    return (valor - etiqueta.a) / (etiqueta.b - etiqueta.a);
  }
  if (valor <= etiqueta.c) {
    return 1;
  }
  if (valor < etiqueta.d) {
    return (etiqueta.d - valor) / (etiqueta.d - etiqueta.c);
  }
  return 0;
}

// La adaptación de un valor a una condición, como difuso::Dominio::adaptacion: recorre sus
// etiquetas en orden; la de la primera a la que se adapta el valor y, si se adapta a otra más, 1
// (supone que las etiquetas forman una partición: la unión de dos que se solapan cubre el valor por
// completo).
double adaptacion(double valor, const Condicion& condicion, const Variable& atributo) {
  std::vector<std::size_t> etiquetas = condicion.etiquetas;
  std::sort(etiquetas.begin(), etiquetas.end());
  etiquetas.erase(std::unique(etiquetas.begin(), etiquetas.end()), etiquetas.end());
  double resultado = 0;
  for (const std::size_t etiqueta : etiquetas) {
    const double grado = adaptacion(valor, atributo.etiquetas[etiqueta]);
    if (resultado == 0) {
      resultado = grado;
    } else if (grado > 0) {
      resultado = 1;
    }
    if (resultado >= 1) {
      break;
    }
  }
  return resultado;
}

bool es_desconocido(double valor) { return std::isnan(valor) || valor == desconocido; }

std::size_t comparar(double x, double y) {
  if (x < y) {
    return comparacion::menor;
  }
  return x == y ? comparacion::igual : comparacion::mayor;
}

// La adaptación de dos valores a una comparación: 1 si su resultado es uno de los de la condición
// y 0 si no (como el atributo que la representa al aprender, con una etiqueta por resultado).
double adaptacion(double x, double y, const Condicion& condicion) {
  if (es_desconocido(x) || es_desconocido(y)) {
    return 1;
  }
  const std::size_t resultado = comparar(x, y);
  const bool cumple = std::find(condicion.etiquetas.begin(), condicion.etiquetas.end(),
                                resultado) != condicion.etiquetas.end();
  return cumple ? 1 : 0;
}

// La adaptación de un ejemplo a una condición.
double adaptacion(const Condicion& condicion, const Datos& datos, std::size_t ejemplo) {
  const double valor = datos.valores[condicion.atributo][ejemplo];
  if (condicion.comparado_con) {
    return adaptacion(valor, datos.valores[*condicion.comparado_con][ejemplo], condicion);
  }
  return adaptacion(valor, condicion, datos.atributos[condicion.atributo]);
}

// La adaptación de un ejemplo a una regla: la menor de sus condiciones
// (difuso::Variables::adaptacion).
double adaptacion(const Regla& regla, const Datos& datos, std::size_t ejemplo) {
  double menor = 1;
  for (const Condicion& condicion : regla.antecedente) {
    menor = std::min(menor, adaptacion(condicion, datos, ejemplo));
    if (menor <= 0) {
      return 0;
    }
  }
  return menor;
}

void comprobar(const std::vector<Regla>& reglas, const Datos& datos) {
  if (datos.valores.size() != datos.atributos.size()) {
    throw std::invalid_argument("inslv: hay " + std::to_string(datos.atributos.size()) +
                                " atributos y " + std::to_string(datos.valores.size()) +
                                " columnas de valores");
  }
  for (std::size_t i = 1; i < datos.valores.size(); ++i) {
    if (datos.valores[i].size() != datos.valores[0].size()) {
      throw std::invalid_argument("inslv: el atributo '" + datos.atributos[i].nombre + "' tiene " +
                                  std::to_string(datos.valores[i].size()) + " valores y '" +
                                  datos.atributos[0].nombre + "', " +
                                  std::to_string(datos.valores[0].size()));
    }
  }
  for (std::size_t r = 0; r < reglas.size(); ++r) {
    for (const Condicion& condicion : reglas[r].antecedente) {
      if (condicion.atributo >= datos.atributos.size()) {
        throw std::invalid_argument("inslv: la regla " + std::to_string(r) + " usa el atributo " +
                                    std::to_string(condicion.atributo) + ", que no existe");
      }
      const Variable& atributo = datos.atributos[condicion.atributo];
      if (condicion.comparado_con) {
        if (*condicion.comparado_con >= datos.atributos.size()) {
          throw std::invalid_argument("inslv: la regla " + std::to_string(r) +
                                      " compara con el atributo " +
                                      std::to_string(*condicion.comparado_con) + ", que no existe");
        }
        for (const std::size_t resultado : condicion.etiquetas) {
          if (resultado > comparacion::mayor) {
            throw std::invalid_argument("inslv: la regla " + std::to_string(r) +
                                        " usa el resultado " + std::to_string(resultado) +
                                        " de una comparación, que no existe");
          }
        }
        continue;
      }
      for (const std::size_t etiqueta : condicion.etiquetas) {
        if (etiqueta >= atributo.etiquetas.size()) {
          throw std::invalid_argument("inslv: la regla " + std::to_string(r) + " usa la etiqueta " +
                                      std::to_string(etiqueta) + " de '" + atributo.nombre +
                                      "', que no existe");
        }
      }
    }
  }
}

}  // namespace

std::vector<std::optional<std::size_t>> clasificar(const std::vector<Regla>& reglas,
                                                   const Datos& datos) {
  comprobar(reglas, datos);
  const std::size_t ejemplos = datos.valores.empty() ? 0 : datos.valores[0].size();
  std::vector<std::optional<std::size_t>> clases(ejemplos);
  for (std::size_t e = 0; e < ejemplos; ++e) {
    // Como reglas::ConjuntoDeReglas::inferir: la primera con la mayor adaptación por peso, salvo
    // que otra con la misma y más peso.
    double mejor = 0;
    const Regla* ganadora = nullptr;
    for (const Regla& regla : reglas) {
      const double grado = adaptacion(regla, datos, e) * regla.peso;
      if (grado > mejor ||
          (ganadora != nullptr && grado > 0 && grado == mejor && regla.peso > ganadora->peso)) {
        mejor = grado;
        ganadora = &regla;
      }
    }
    if (ganadora != nullptr) {
      clases[e] = ganadora->consecuente;
    }
  }
  return clases;
}

}  // namespace inslv
