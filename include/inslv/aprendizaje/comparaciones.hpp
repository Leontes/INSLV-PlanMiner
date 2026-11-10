// SPDX-License-Identifier: GPL-3.0-only
// Las comparaciones entre atributos (Opciones::comparaciones). Cada par de atributos de la misma
// unidad da un atributo nuevo, el resultado de compararlos (menor, igual o mayor), que se queda si
// da información sobre la clase. Es lo que el original preparaba y tenía apagado: atributos nuevos
// con funciones de pares de atributos de la misma unidad, elegidos por su información
// (MedidasDeInformacion_operadores en old/).
#pragma once

#include <cstddef>
#include <vector>

#include "inslv/inslv.hpp"

namespace inslv::aprendizaje {

/// Dos atributos que se comparan, por su posición en Datos::atributos (primero < segundo).
struct Par {
  std::size_t primero = 0;
  std::size_t segundo = 0;
};

/// Los pares de atributos con la misma unidad, distinta de 0, en orden.
[[nodiscard]] std::vector<Par> pares_de_la_misma_unidad(const Datos& datos);

/// El atributo que compara el par: sus etiquetas son los resultados (comparacion::menor, igual y
/// mayor), y sus valores, esos mismos números.
[[nodiscard]] Variable variable_de_comparacion(const Datos& datos, const Par& par);

/// Los valores de ese atributo en cada ejemplo: el resultado de comparar el primero con el
/// segundo, o NaN si alguno es desconocido.
[[nodiscard]] std::vector<double> valores_de_comparacion(const Datos& datos, const Par& par);

/// Los pares cuya comparación da información sobre alguna clase: más de umbral por la mayor de los
/// atributos de los datos con esa clase (la información mutua normalizada de
/// medidas_de_informacion, con todos los ejemplos).
[[nodiscard]] std::vector<Par> elegir_comparaciones(const Datos& datos,
                                                    const std::vector<Par>& pares, double umbral);

/// Los datos con la comparación de cada par como atributo, después de los suyos.
[[nodiscard]] Datos con_comparaciones(const Datos& datos, const std::vector<Par>& pares);

/// Pasa las reglas aprendidas con con_comparaciones(datos, pares) a los atributos de datos: las
/// condiciones sobre las comparaciones (los atributos desde atributos, el número de los de datos)
/// quedan como condiciones con Condicion::comparado_con.
void traducir_comparaciones(std::vector<Regla>& reglas, std::size_t atributos,
                            const std::vector<Par>& pares);

}  // namespace inslv::aprendizaje
