// SPDX-License-Identifier: GPL-3.0-only
// Los patrones de las reglas: una regla descriptiva y su discriminante, juntas, dicen qué etiquetas
// son imprescindibles y cuáles indiferentes. Con ellos se combinan las reglas compatibles.
#pragma once

#include <string>
#include <vector>

#include "inslv/difuso/VectorVar.h"
#include "inslv/reglas/ruleset.h"

namespace inslv {

/// Un aviso de consistencia, al nivel de detalle.
void Pausa();
/// Las diferencias entre dos patrones, a la depuración.
void Comparar2patronesPinta(const std::string& cad1, const std::string& cad2);
/// Si esp es una especificación de gen: no admite ninguna etiqueta que gen no admita.
[[nodiscard]] bool ComprobacionEspGen(const std::string& esp, const std::string& gen);
/// Las etiquetas de la regla pos, o una cadena vacía si no existe.
[[nodiscard]] std::string Patron1Regla(const ruleset& R_1, int pos);
/// El patrón de dos reglas emparejadas: sus etiquetas donde coinciden y '?' donde no.
[[nodiscard]] std::string Patron2Reglas(const ruleset& R_1, const ruleset& R_2, const VectorVar& V,
                                        int pos);
/// Quita las reglas de los tres conjuntos, emparejados. reglas_a_eliminar va en orden creciente.
void Filtrar_Reglas_New(ruleset& R, ruleset& R_dis, ruleset& R_mix,
                        const std::vector<int>& reglas_a_eliminar);
/// Combina las nuevas (las primeras) con las demás reglas de su clase que tengan un patrón
/// compatible, y resta peso a las que tengan el mismo patrón que una de otra clase.
void AnalizarConjuntosDeReglas(const VectorVar& V, ruleset& R, ruleset& R_Descrip, ruleset& R_mix,
                               int nuevas);

}  // namespace inslv
