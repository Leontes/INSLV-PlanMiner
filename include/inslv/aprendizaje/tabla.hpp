// SPDX-License-Identifier: GPL-3.0-only
// La tabla con las adaptaciones de los ejemplos a las etiquetas de las variables.
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "inslv/datos/ejemplos.hpp"
#include "inslv/difuso/variables.hpp"

namespace inslv::aprendizaje {

/// Se calcula una vez para unos ejemplos y evita repetir la cuenta al valorar cada individuo, que
/// es casi todo el tiempo del aprendizaje. Al afinar una regla solo cambia un atributo cada vez,
/// así que guarda también la adaptación a cada conjunto de etiquetas que le piden.
///
/// Por eso, aunque sus métodos sean const, no se puede usar a la vez desde varios hilos.
class TablaDeAdaptaciones {
 public:
  /// memoria_maxima: los bytes para los conjuntos de etiquetas que guarda; al llegar a ellos, los
  /// olvida. Lanza std::invalid_argument si algún valor no se adapta a ninguna etiqueta de su
  /// variable.
  TablaDeAdaptaciones(const difuso::Variables& variables, const datos::Ejemplos& ejemplos,
                      std::size_t memoria_maxima = std::size_t{256} * 1024 * 1024);

  /// La adaptación del ejemplo a una etiqueta de la variable.
  [[nodiscard]] double adaptacion_a_etiqueta(int ejemplo, int variable, int etiqueta) const;
  /// La adaptación de cada ejemplo a una clase.
  [[nodiscard]] const std::vector<double>& adaptaciones_a_clase(int clase) const;
  /// La adaptación de cada ejemplo a las etiquetas de la variable marcadas con '1': la de la
  /// primera con adaptación positiva o, si son dos, 1. La referencia deja de valer en la siguiente
  /// llamada.
  [[nodiscard]] const std::vector<double>& adaptaciones_a_variable(
      int variable, std::string_view etiquetas) const;
  /// En salida, la adaptación de cada ejemplo a un antecedente (ver genetico::Codigo): la menor de
  /// los atributos con nivel por encima del umbral.
  void adaptaciones_al_antecedente(const std::string& etiquetas, const std::vector<double>& niveles,
                                   std::vector<double>& salida) const;
  /// La primera etiqueta de la variable con la mayor adaptación del ejemplo.
  [[nodiscard]] int mejor_etiqueta(int ejemplo, int variable) const;

 private:
  // Para buscar con un string_view sin crear una cadena.
  struct HashDeCadena {
    using is_transparent = void;
    std::size_t operator()(std::string_view cadena) const {
      return std::hash<std::string_view>{}(cadena);
    }
  };
  using ConjuntosDeEtiquetas =
      std::unordered_map<std::string, std::vector<double>, HashDeCadena, std::equal_to<>>;

  // adaptaciones_[variable][etiqueta][ejemplo]
  std::vector<std::vector<std::vector<double>>> adaptaciones_;
  std::size_t n_ejemplos_;
  int n_atributos_;
  int consecuente_;
  // conjuntos_[variable][etiquetas][ejemplo]
  mutable std::vector<ConjuntosDeEtiquetas> conjuntos_;
  mutable std::size_t conjuntos_guardados_ = 0;
  std::size_t conjuntos_maximos_;
};

}  // namespace inslv::aprendizaje
