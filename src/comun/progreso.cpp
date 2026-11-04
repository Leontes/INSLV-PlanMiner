// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/comun/progreso.hpp"

#include <streambuf>

namespace inslv::comun {

namespace {

// Un búfer que descarta lo que se escribe.
class BuferNulo : public std::streambuf {
 protected:
  int overflow(int caracter) override { return traits_type::not_eof(caracter); }
};

// Por hilo: el destino del progreso, su verbosidad y el flujo que lo descarta (los flujos guardan
// estado, como la precisión, y no se pueden compartir entre hilos).
thread_local BuferNulo bufer_nulo;
thread_local std::ostream ninguna_parte(&bufer_nulo);
thread_local std::ostream* destino = nullptr;
thread_local Verbosidad verbosidad = Verbosidad::ninguna;

}  // namespace

bool se_escribe(Verbosidad nivel) {
  return destino != nullptr && nivel != Verbosidad::ninguna && nivel <= verbosidad;
}

std::ostream& progreso(Verbosidad nivel) { return se_escribe(nivel) ? *destino : ninguna_parte; }

DestinoDelProgreso::DestinoDelProgreso(std::ostream* flujo, Verbosidad nueva_verbosidad)
    : flujo_anterior_(destino), verbosidad_anterior_(verbosidad) {
  destino = flujo;
  verbosidad = nueva_verbosidad;
}

DestinoDelProgreso::~DestinoDelProgreso() {
  destino = flujo_anterior_;
  verbosidad = verbosidad_anterior_;
}

}  // namespace inslv::comun
