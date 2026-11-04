// SPDX-License-Identifier: GPL-3.0-only
// Lo que cuenta la implementación mientras aprende (los episodios, el genético...), por niveles
// de verbosidad (inslv::Verbosidad).
//
// Va al flujo que fija aprender para el hilo que aprende (Opciones::progreso, o la salida
// estándar), con la verbosidad de las opciones: un mensaje solo se escribe si la verbosidad llega
// a su nivel. Antes se desviaba std::cout, que es de todo el programa.
#pragma once

#include <ostream>

#include "inslv/inslv.hpp"

namespace inslv::comun {

/// El flujo para un mensaje de ese nivel en este hilo: el del destino si la verbosidad llega al
/// nivel; si no, uno que lo descarta.
std::ostream& progreso(Verbosidad nivel);

/// Si un mensaje de ese nivel se escribe en este hilo (para no prepararlo si no).
bool se_escribe(Verbosidad nivel);

/// Mientras existe, el progreso de este hilo va a flujo (nullptr: a ninguna parte) con esa
/// verbosidad. Al destruirse, vuelve el destino que hubiera.
class DestinoDelProgreso {
 public:
  DestinoDelProgreso(std::ostream* flujo, Verbosidad nueva_verbosidad);
  ~DestinoDelProgreso();
  DestinoDelProgreso(const DestinoDelProgreso&) = delete;
  DestinoDelProgreso& operator=(const DestinoDelProgreso&) = delete;
  DestinoDelProgreso(DestinoDelProgreso&&) = delete;
  DestinoDelProgreso& operator=(DestinoDelProgreso&&) = delete;

 private:
  std::ostream* flujo_anterior_;
  Verbosidad verbosidad_anterior_;
};

}  // namespace inslv::comun
