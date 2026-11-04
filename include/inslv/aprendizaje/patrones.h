// SPDX-License-Identifier: GPL-3.0-only
// Los patrones de las reglas: una regla descriptiva y su discriminante, juntas, dicen qué etiquetas
// son imprescindibles y cuáles indiferentes. Con ellos se combinan las reglas compatibles.
#pragma once

#include <string>
#include <vector>

#include "inslv/difuso/variables.hpp"
#include "inslv/reglas/conjunto_de_reglas.hpp"

namespace inslv {

/// Un aviso de consistencia, al nivel de detalle.
void Pausa();
/// Las diferencias entre dos patrones, a la depuración.
void Comparar2patronesPinta(const std::string& cad1, const std::string& cad2);
/// Si esp es una especificación de gen: no admite ninguna etiqueta que gen no admita.
[[nodiscard]] bool ComprobacionEspGen(const std::string& esp, const std::string& gen);
/// Las etiquetas de la regla pos, o una cadena vacía si no existe.
[[nodiscard]] std::string Patron1Regla(const reglas::ConjuntoDeReglas& R_1, int pos);
/// El patrón de dos reglas emparejadas: sus etiquetas donde coinciden y '?' donde no.
[[nodiscard]] std::string Patron2Reglas(const reglas::ConjuntoDeReglas& R_1,
                                        const reglas::ConjuntoDeReglas& R_2,
                                        const difuso::Variables& V, int pos);
/// Quita las reglas de los tres conjuntos, emparejados. reglas_a_eliminar va en orden creciente.
void Filtrar_Reglas_New(reglas::ConjuntoDeReglas& R, reglas::ConjuntoDeReglas& R_dis,
                        reglas::ConjuntoDeReglas& R_mix, const std::vector<int>& reglas_a_eliminar);
/// Combina las nuevas (las primeras) con las demás reglas de su clase que tengan un patrón
/// compatible, y resta peso a las que tengan el mismo patrón que una de otra clase.
void AnalizarConjuntosDeReglas(const difuso::Variables& V, reglas::ConjuntoDeReglas& R,
                               reglas::ConjuntoDeReglas& R_Descrip, reglas::ConjuntoDeReglas& R_mix,
                               int nuevas);

}  // namespace inslv
