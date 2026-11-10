// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/comparaciones.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "inslv/aprendizaje/aprender.hpp"
#include "inslv/aprendizaje/medidas.hpp"
#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"

namespace inslv::aprendizaje {

namespace {

double resultado(std::size_t comparacion) { return static_cast<double>(comparacion); }

double comparar(double x, double y) {
  if (std::isnan(x) || std::isnan(y)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  if (x < y) {
    return resultado(comparacion::menor);
  }
  return x == y ? resultado(comparacion::igual) : resultado(comparacion::mayor);
}

// La información de cada atributo con todas las clases (columna 0) y con cada una.
std::vector<std::vector<double>> informacion(const Datos& datos) {
  const difuso::Variables variables(datos);
  const datos::Ejemplos ejemplos = ejemplos_de(datos);
  return medidas_de_informacion(variables, ejemplos);
}

}  // namespace

std::vector<Par> pares_de_la_misma_unidad(const Datos& datos) {
  std::vector<Par> pares;
  for (std::size_t i = 0; i < datos.atributos.size(); ++i) {
    if (datos.atributos[i].unidad == 0) {
      continue;
    }
    for (std::size_t j = i + 1; j < datos.atributos.size(); ++j) {
      if (datos.atributos[j].unidad == datos.atributos[i].unidad) {
        pares.push_back({i, j});
      }
    }
  }
  return pares;
}

Variable variable_de_comparacion(const Datos& datos, const Par& par) {
  const auto etiqueta = [](const char* nombre, std::size_t comparacion) {
    const double x = resultado(comparacion);
    return ConjuntoDifuso{nombre, x, x, x, x};
  };
  Variable variable;
  variable.nombre =
      datos.atributos[par.primero].nombre + " frente a " + datos.atributos[par.segundo].nombre;
  variable.etiquetas = {etiqueta("menor", comparacion::menor),
                        etiqueta("igual", comparacion::igual),
                        etiqueta("mayor", comparacion::mayor)};
  return variable;
}

std::vector<double> valores_de_comparacion(const Datos& datos, const Par& par) {
  const std::vector<double>& primero = datos.valores[par.primero];
  const std::vector<double>& segundo = datos.valores[par.segundo];
  std::vector<double> valores;
  valores.reserve(primero.size());
  for (std::size_t e = 0; e < primero.size(); ++e) {
    valores.push_back(comparar(primero[e], segundo[e]));
  }
  return valores;
}

std::vector<Par> elegir_comparaciones(const Datos& datos, const std::vector<Par>& pares,
                                      double umbral) {
  if (pares.empty()) {
    return {};
  }
  // La mayor información de los atributos con cada clase.
  std::vector<double> mayor(datos.clase.etiquetas.size(), 0);
  for (const std::vector<double>& atributo : informacion(datos)) {
    for (std::size_t k = 0; k < mayor.size(); ++k) {
      mayor[k] = std::max(mayor[k], atributo[k + 1]);
    }
  }

  // Cada comparación, sola con la clase: así no hace falta tenerlas todas a la vez.
  Datos una;
  una.clase = datos.clase;
  una.clases = datos.clases;
  std::vector<Par> elegidas;
  for (const Par& par : pares) {
    una.atributos = {variable_de_comparacion(datos, par)};
    una.valores = {valores_de_comparacion(datos, par)};
    const std::vector<double> suya = informacion(una).front();
    for (std::size_t k = 0; k < mayor.size(); ++k) {
      if (suya[k + 1] > umbral * mayor[k]) {
        elegidas.push_back(par);
        break;
      }
    }
  }
  return elegidas;
}

Datos con_comparaciones(const Datos& datos, const std::vector<Par>& pares) {
  Datos resultado = datos;
  resultado.atributos.reserve(datos.atributos.size() + pares.size());
  resultado.valores.reserve(datos.valores.size() + pares.size());
  for (const Par& par : pares) {
    resultado.atributos.push_back(variable_de_comparacion(datos, par));
    resultado.valores.push_back(valores_de_comparacion(datos, par));
  }
  return resultado;
}

void traducir_comparaciones(std::vector<Regla>& reglas, std::size_t atributos,
                            const std::vector<Par>& pares) {
  for (Regla& regla : reglas) {
    for (Condicion& condicion : regla.antecedente) {
      if (condicion.atributo >= atributos) {
        const Par& par = pares[condicion.atributo - atributos];
        condicion.atributo = par.primero;
        condicion.comparado_con = par.segundo;
      }
    }
  }
}

}  // namespace inslv::aprendizaje
