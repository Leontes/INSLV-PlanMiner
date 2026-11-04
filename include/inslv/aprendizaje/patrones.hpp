// SPDX-License-Identifier: GPL-3.0-only
// Los patrones de las reglas: una regla descriptiva y su discriminante, juntas, dicen qué etiquetas
// son imprescindibles y cuáles indiferentes. Con ellos se combinan las reglas compatibles.
#pragma once

#include <string>
#include <vector>

#include "inslv/difuso/variables.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv::aprendizaje {

/// Un aviso de consistencia, al nivel de detalle.
void aviso_de_consistencia();
/// Las diferencias entre dos patrones, a la depuración.
void pintar_diferencias(const std::string& patron1, const std::string& patron2);
/// Si esp es una especificación de gen: no admite ninguna etiqueta que gen no admita.
[[nodiscard]] bool es_especificacion(const std::string& esp, const std::string& gen);
/// Las etiquetas de la regla, o una cadena vacía si no existe.
[[nodiscard]] std::string patron_de_regla(const reglas::ConjuntoDeReglas& reglas, int regla);
/// El patrón de dos reglas emparejadas: sus etiquetas donde coinciden y '?' donde no.
[[nodiscard]] std::string patron_de_dos_reglas(const reglas::ConjuntoDeReglas& reglas1,
                                               const reglas::ConjuntoDeReglas& reglas2,
                                               const difuso::Variables& variables, int regla);
/// Quita las reglas de los tres conjuntos, emparejados. reglas_a_eliminar va en orden creciente.
void quitar_reglas(reglas::ConjuntoDeReglas& reglas1, reglas::ConjuntoDeReglas& reglas2,
                   reglas::ConjuntoDeReglas& reglas3, const std::vector<int>& reglas_a_eliminar);
/// Combina las nuevas (las primeras) con las demás reglas de su clase que tengan un patrón
/// compatible, y resta peso a las que tengan el mismo patrón que una de otra clase.
void combinar_reglas(const difuso::Variables& variables, reglas::ConjuntoDeReglas& discriminantes,
                     reglas::ConjuntoDeReglas& descriptivas, reglas::ConjuntoDeReglas& patrones,
                     int nuevas);

}  // namespace inslv::aprendizaje
