// SPDX-License-Identifier: GPL-3.0-only
// Lectura de conjuntos de datos en el formato de NSLV.
//
// Un conjunto de datos tiene un dominio (.dom) con las variables y sus etiquetas, y uno o varios
// ficheros de ejemplos (.datos), por ejemplo las particiones de una validación cruzada.
//
// Dominio: el número de variables y, por cada una, su nombre, si es la clase (1) o un atributo
// (0), la unidad (Variable::unidad; 0, ninguna), un campo que no se usa (el factor), el mínimo, el
// máximo, el número de etiquetas y cada etiqueta (a b c d nombre). Ejemplos: el número de ejemplos,
// el número de variables y un ejemplo por fila, con los valores en el orden del dominio. La clase
// se escribe como el centro de su etiqueta; un valor desconocido, como -999999999. Todo se separa
// con espacios en blanco.
#pragma once

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "inslv/inslv.hpp"

namespace inslv::nslv {

/// Un fichero que no sigue el formato. El mensaje dice el fichero y la línea.
class ErrorDeFormato : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

/// Lee un dominio: los atributos, en el orden del fichero, y la clase, que tiene que ser una sola
/// variable. El conjunto que devuelve no tiene ejemplos. Lanza ErrorDeFormato si el fichero no
/// sigue el formato o una etiqueta no cumple a <= b <= c <= d.
Datos leer_dominio(const std::filesystem::path& dominio);

/// Lee un dominio con los ejemplos de varios ficheros, uno tras otro. Los desconocidos quedan como
/// NaN. Lanza ErrorDeFormato si un fichero no sigue el formato, no tiene las variables del dominio
/// o tiene una clase que no es el centro de ninguna de sus etiquetas.
Datos leer(const std::filesystem::path& dominio,
           const std::vector<std::filesystem::path>& ejemplos);

}  // namespace inslv::nslv
