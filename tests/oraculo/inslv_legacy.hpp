// SPDX-License-Identifier: GPL-3.0-only
// La cabecera de old/ con su espacio de nombres cambiado: inslv_legacy en vez de inslv.
//
// El target inslv_legacy compila las fuentes de old/ con la misma macro (cmake/Oraculo.cmake),
// así que esta cabecera declara lo que define. Se puede incluir junto a inslv/inslv.hpp.
#pragma once

// Las cabeceras estándar que usa la de old/, antes de la macro.
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#define inslv inslv_legacy
#include INSLV_CABECERA_LEGACY
#undef inslv
