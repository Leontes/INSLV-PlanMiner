// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/comun/progreso.hpp"

#include <streambuf>

namespace inslv {

namespace {

// Un búfer que descarta lo que se escribe.
class BuferNulo : public std::streambuf {
 protected:
  int overflow(int caracter) override { return traits_type::not_eof(caracter); }
};

// Por hilo: el destino del progreso y el flujo que lo descarta (los flujos guardan estado, como
// la precisión, y no se pueden compartir entre hilos).
thread_local BuferNulo bufer_nulo;
thread_local std::ostream ninguna_parte(&bufer_nulo);
thread_local std::ostream* destino = nullptr;

}  // namespace

std::ostream& progreso() { return destino != nullptr ? *destino : ninguna_parte; }

DestinoDelProgreso::DestinoDelProgreso(std::ostream* flujo) : anterior_(destino) {
  destino = flujo;
}

DestinoDelProgreso::~DestinoDelProgreso() { destino = anterior_; }

}  // namespace inslv
