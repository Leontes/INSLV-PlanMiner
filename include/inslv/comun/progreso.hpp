// SPDX-License-Identifier: GPL-3.0-only
// El progreso que escribe la implementación mientras aprende (los episodios, el genético...).
//
// Va al flujo que fija learn para el hilo que aprende (Options::progress, o std::cout con verbose)
// o, si no hay ninguno, a ninguna parte. Antes learn desviaba std::cout, que es de todo el
// programa.
#pragma once

#include <ostream>

namespace inslv {

/// El flujo del progreso en este hilo.
std::ostream& progreso();

/// Mientras existe, el progreso de este hilo va a flujo (nullptr: a ninguna parte). Al destruirse,
/// vuelve el destino que hubiera.
class DestinoDelProgreso {
 public:
  explicit DestinoDelProgreso(std::ostream* flujo);
  ~DestinoDelProgreso();
  DestinoDelProgreso(const DestinoDelProgreso&) = delete;
  DestinoDelProgreso& operator=(const DestinoDelProgreso&) = delete;
  DestinoDelProgreso(DestinoDelProgreso&&) = delete;
  DestinoDelProgreso& operator=(DestinoDelProgreso&&) = delete;

 private:
  std::ostream* anterior_;
};

}  // namespace inslv
