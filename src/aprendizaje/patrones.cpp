// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/patrones.h"

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <ranges>
#include <string>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// 0 si los patrones son iguales; 1 si son compatibles (solo difieren en algún '?'); 2 si no.
int Comparar2patrones(const std::string& cad1, const std::string& cad2) {
  int n_cambios = 0;
  bool compatibles = true;
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    if (cad1[s] != cad2[s]) {
      if ((cad1[s] == '0' && cad2[s] == '1') || (cad1[s] == '1' && cad2[s] == '0')) {
        compatibles = false;
      }
      ++n_cambios;
    }
  }
  if (n_cambios == 0) {
    return 0;
  }
  return compatibles ? 1 : 2;
}

// Las etiquetas de la regla, con los atributos por debajo del umbral admitiendo todas.
std::string EtiquetasEfectivas(const genetico::Codigo& code, const difuso::Variables& V) {
  std::string etiquetas = code.etiquetas;
  std::size_t tama = 0;
  for (int j = 0; j < V.numero_de_atributos(); ++j) {
    const auto longitud = indice(V.numero_de_etiquetas(j));
    if (code.niveles[indice(j)] < code.niveles.back()) {
      etiquetas.replace(tama, longitud, longitud, '1');
    }
    tama += longitud;
  }
  return etiquetas;
}

std::string Union(const std::string& cad1, const std::string& cad2) {
  std::string aux = cad1;
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    aux[s] = cad1[s] == '0' && cad2[s] == '0' ? '0' : '1';
  }
  return aux;
}

std::string Interseccion(const std::string& cad1, const std::string& cad2) {
  std::string aux = cad1;
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    aux[s] = cad1[s] == '1' && cad2[s] == '1' ? '1' : '0';
  }
  return aux;
}

// Hace la union de las dos reglas y la operacion la almacena en la regla r1
void UnionReglasDescriptivas(const difuso::Variables& V, reglas::ConjuntoDeReglas& R1, int r1,
                             const reglas::ConjuntoDeReglas& R2, int r2) {
  const std::string patron =
      Union(EtiquetasEfectivas(R1.codigo(r1), V), EtiquetasEfectivas(R2.codigo(r2), V));
  R1.codigo(r1).etiquetas = patron;
}

// Hace la interseccion de las dos reglas y la operacion la almacena en la regla r1
void IntersecionReglasDiscriminantes(const difuso::Variables& V, reglas::ConjuntoDeReglas& R1,
                                     int r1, const reglas::ConjuntoDeReglas& R2, int r2) {
  const std::string patron =
      Interseccion(EtiquetasEfectivas(R1.codigo(r1), V), EtiquetasEfectivas(R2.codigo(r2), V));
  genetico::Codigo& code = R1.codigo(r1);
  code.etiquetas = patron;
  for (int j = 0; j < V.numero_de_atributos(); ++j) {
    if (code.niveles[indice(j)] < code.niveles.back()) {
      code.niveles[indice(j)] = code.niveles.back();
    }
  }
}

int Grados_Libertad(const std::string& cad) {
  return static_cast<int>(std::count(cad.begin(), cad.end(), '?'));
}

std::string Patron2Patrones(const std::string& pat1, const std::string& pat2) {
  std::string pat3 = pat1;
  for (std::size_t i = 0; i < pat1.size(); ++i) {
    if (pat1[i] == pat2[i]) {
      pat3[i] = pat1[i];
    } else if ((pat1[i] == '1' && pat2[i] == '?') || (pat1[i] == '?' && pat2[i] == '1')) {
      pat3[i] = '1';
    } else if ((pat1[i] == '0' && pat2[i] == '?') || (pat1[i] == '?' && pat2[i] == '0')) {
      pat3[i] = '0';
    } else {
      pat3[i] = '?';
    }
  }
  return pat3;
}

}  // namespace

// En el original esperaba una tecla tras los avisos de consistencia del análisis de patrones.
// Como librería no puede bloquearse esperando la entrada estándar: solo avisa.
void Pausa() {
  comun::progreso(Verbosidad::detalle)
      << "INSLV: aviso de consistencia en el análisis de patrones (ver la salida anterior)\n";
}

// Las diferencias entre dos patrones, a la depuración.
void Comparar2patronesPinta(const std::string& cad1, const std::string& cad2) {
  std::ostream& salida = comun::progreso(Verbosidad::depuracion);
  for (std::size_t s = 0; s < cad1.size(); ++s) {
    if (cad1[s] == cad2[s]) {
      salida << '_';
    } else if (cad1[s] == '0' && cad2[s] == '1') {
      salida << '>';
    } else if (cad1[s] == '1' && cad2[s] == '0') {
      salida << '<';
    } else {
      salida << '$';
    }
  }
  salida << '\n';
}

// Determina si se mantiene que esp es una especificacion de gen.
bool ComprobacionEspGen(const std::string& esp, const std::string& gen) {
  for (std::size_t i = 0; i < esp.size(); ++i) {
    if (esp[i] == '1' && gen[i] == '0') {
      return false;
    }
  }
  return true;
}

std::string Patron1Regla(const reglas::ConjuntoDeReglas& R_1, int pos) {
  // Sin esa regla, un patrón vacío. En el original leía la regla -1 cuando el genético no
  // añadía ninguna en la primera vuelta (R_mix vacío) y el programa reventaba (HALLAZGOS).
  if (pos < 0 || pos >= R_1.numero_de_reglas()) {
    return "";
  }
  return R_1.codigo(pos).etiquetas;
}

std::string Patron2Reglas(const reglas::ConjuntoDeReglas& R_1, const reglas::ConjuntoDeReglas& R_2,
                          const difuso::Variables& V, int pos) {
  const std::string regla1 = EtiquetasEfectivas(R_1.codigo(pos), V);
  const std::string regla2 = EtiquetasEfectivas(R_2.codigo(pos), V);
  std::string regla3 = regla1;
  for (std::size_t i = 0; i < regla1.size(); ++i) {
    if (regla1[i] != regla2[i]) {
      regla3[i] = '?';
    }
  }
  return regla3;
}

void Filtrar_Reglas_New(reglas::ConjuntoDeReglas& R, reglas::ConjuntoDeReglas& R_dis,
                        reglas::ConjuntoDeReglas& R_mix,
                        const std::vector<int>& reglas_a_eliminar) {
  for (const int regla : std::views::reverse(reglas_a_eliminar)) {
    R.quitar(regla);
    R_dis.quitar(regla);
    R_mix.quitar(regla);
  }
}

void AnalizarConjuntosDeReglas(const difuso::Variables& V, reglas::ConjuntoDeReglas& R,
                               reglas::ConjuntoDeReglas& R_Descrip, reglas::ConjuntoDeReglas& R_mix,
                               int nuevas) {
  std::vector<bool> eliminar(indice(R.numero_de_reglas()), false);

  for (int i = 0; i < nuevas; ++i) {
    if (eliminar[indice(i)]) {
      continue;
    }
    const std::string res = Patron2Reglas(R_Descrip, R, V, i);
    const std::string res3 = Patron1Regla(R_Descrip, i);
    const std::string res4 = Patron1Regla(R, i);
    const std::string patr = R_mix.codigo(i).etiquetas;

    if (res != patr) {
      comun::progreso(Verbosidad::depuracion)
          << "No coinciden los patrones para la regla " << i << '\n'
          << res << '\n'
          << patr << '\n';
      Pausa();
    }

    if (res4 != patr && Comparar2patrones(patr, res4) != 1) {
      comun::progreso(Verbosidad::depuracion)
          << "La regla discriminante " << i << " no coincide con su patron\n"
          << res4 << '\n'
          << patr << '\n';
      Pausa();
    }

    if (res3 != patr && Comparar2patrones(patr, res3) != 1) {
      comun::progreso(Verbosidad::depuracion)
          << "La regla descriptiva " << i << " no coincide con su patron\n"
          << res3 << '\n'
          << patr << '\n';
      Pausa();
    }

    for (int j = i + 1; j < R.numero_de_reglas() && !eliminar[indice(i)]; ++j) {
      if (eliminar[indice(j)]) {
        continue;
      }
      const std::string res2 = Patron2Reglas(R_Descrip, R, V, j);
      const int iguales = Comparar2patrones(res2, patr);

      if (R.clase(i) != R.clase(j)) {
        if (iguales == 0) {
          comun::progreso(Verbosidad::detalle)
              << "las reglas " << i << " y " << j << " son contradictorias\n";
          R.sumar_fallos(i, R.aciertos(i));
          R.recalcular_peso(i);
          R_Descrip.sumar_fallos(i, R_Descrip.aciertos(i));
          R_Descrip.recalcular_peso(i);
        } else if (iguales == 1) {
          comun::progreso(Verbosidad::detalle)
              << "las reglas " << i << " y " << j << " entran en conflicto\n";
        }
        continue;
      }

      // El caso raro de compatibilidades entre patrones
      if (iguales > 1 && Comparar2patrones(res3, res2) != 2 && Comparar2patrones(res4, res2) != 2) {
        comun::progreso(Verbosidad::depuracion)
            << "Caso especial de incompatibilidades entre patrones y si entre reglas\n"
            << res3 << '\n'
            << res4 << '\n'
            << patr << "\n\n"
            << Patron1Regla(R_Descrip, j) << '\n'
            << Patron1Regla(R, j) << '\n'
            << res2 << "\n\n";
        Pausa();
      }

      if (iguales == 0) {
        // Los dos patrones son iguales
        eliminar[indice(i)] = true;
      } else if (iguales == 1) {
        // Los dos patrones son compatibles
        comun::progreso(Verbosidad::detalle) << "\t\t\t\t\tCombinando Reglas " << i << " y " << j
                                             << " de la clase " << R.clase(i) << '\n';

        const std::string patron_i = Patron2Reglas(R_Descrip, R, V, i);
        const std::string patron_j = Patron2Reglas(R_Descrip, R, V, j);

        IntersecionReglasDiscriminantes(V, R, j, R, i);
        UnionReglasDescriptivas(V, R_Descrip, j, R_Descrip, i);

        const std::string resultado = Patron2Reglas(R_Descrip, R, V, j);
        const std::string combinado = Patron2Patrones(patron_i, patron_j);
        comun::progreso(Verbosidad::depuracion) << combinado << '\n';
        if (Comparar2patrones(resultado, combinado) != 0) {
          comun::progreso(Verbosidad::depuracion) << resultado << '\n' << combinado << '\n';
          Comparar2patronesPinta(resultado, combinado);
          Pausa();
        }

        R_mix.codigo(j).etiquetas = combinado;
        std::string discriminante = combinado;
        std::replace(discriminante.begin(), discriminante.end(), '?', '1');
        R.codigo(j).etiquetas = discriminante;
        std::string descriptiva = combinado;
        std::replace(descriptiva.begin(), descriptiva.end(), '?', '0');
        R_Descrip.codigo(j).etiquetas = descriptiva;

        if (Patron2Reglas(R_Descrip, R, V, j) != combinado) {
          comun::progreso(Verbosidad::depuracion)
              << "Error al componer la descrip. y la predic. en la regla " << j << '\n';
          Pausa();
        }

        eliminar[indice(i)] = true;
        R_Descrip.marcar_modificada(j, true);
        R.marcar_modificada(j, true);

        if (Grados_Libertad(res2) >= Grados_Libertad(patr)) {
          R_Descrip.sumar_aciertos(j, R_Descrip.aciertos(i));
          R_Descrip.sumar_fallos(j, R_Descrip.fallos(i));
          R.sumar_aciertos(j, R.aciertos(i));
          R.sumar_fallos(j, R.fallos(i));
        }
      }
    }
  }

  std::vector<int> eliminables;
  for (int i = 0; i < R.numero_de_reglas() - 1; ++i) {
    if (eliminar[indice(i)]) {
      eliminables.push_back(i);
    }
  }

  Filtrar_Reglas_New(R, R_Descrip, R_mix, eliminables);
}

}  // namespace inslv
