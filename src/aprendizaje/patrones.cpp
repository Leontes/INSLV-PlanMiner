// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/patrones.hpp"

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <ranges>
#include <string>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// 0 si los patrones son iguales; 1 si son compatibles (solo difieren en algún '?'); 2 si no.
int comparar_patrones(const std::string& patron1, const std::string& patron2) {
  int n_cambios = 0;
  bool compatibles = true;
  for (std::size_t s = 0; s < patron1.size(); ++s) {
    if (patron1[s] != patron2[s]) {
      if ((patron1[s] == '0' && patron2[s] == '1') || (patron1[s] == '1' && patron2[s] == '0')) {
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
std::string etiquetas_efectivas(const genetico::Codigo& codigo,
                                const difuso::Variables& variables) {
  std::string etiquetas = codigo.etiquetas;
  std::size_t tama = 0;
  for (int j = 0; j < variables.numero_de_atributos(); ++j) {
    const auto longitud = indice(variables.numero_de_etiquetas(j));
    if (codigo.niveles[indice(j)] < codigo.niveles.back()) {
      etiquetas.replace(tama, longitud, longitud, '1');
    }
    tama += longitud;
  }
  return etiquetas;
}

std::string union_de_etiquetas(const std::string& patron1, const std::string& patron2) {
  std::string resultado = patron1;
  for (std::size_t s = 0; s < patron1.size(); ++s) {
    resultado[s] = patron1[s] == '0' && patron2[s] == '0' ? '0' : '1';
  }
  return resultado;
}

std::string interseccion_de_etiquetas(const std::string& patron1, const std::string& patron2) {
  std::string resultado = patron1;
  for (std::size_t s = 0; s < patron1.size(); ++s) {
    resultado[s] = patron1[s] == '1' && patron2[s] == '1' ? '1' : '0';
  }
  return resultado;
}

// Hace la union de las dos reglas y la operacion la almacena en la regla regla1
void unir_descriptivas(const difuso::Variables& variables, reglas::ConjuntoDeReglas& reglas1,
                       int regla1, const reglas::ConjuntoDeReglas& reglas2, int regla2) {
  const std::string patron =
      union_de_etiquetas(etiquetas_efectivas(reglas1.codigo(regla1), variables),
                         etiquetas_efectivas(reglas2.codigo(regla2), variables));
  reglas1.codigo(regla1).etiquetas = patron;
}

// Hace la interseccion de las dos reglas y la operacion la almacena en la regla regla1
void intersecar_discriminantes(const difuso::Variables& variables,
                               reglas::ConjuntoDeReglas& reglas1, int regla1,
                               const reglas::ConjuntoDeReglas& reglas2, int regla2) {
  const std::string patron =
      interseccion_de_etiquetas(etiquetas_efectivas(reglas1.codigo(regla1), variables),
                                etiquetas_efectivas(reglas2.codigo(regla2), variables));
  genetico::Codigo& codigo = reglas1.codigo(regla1);
  codigo.etiquetas = patron;
  for (int j = 0; j < variables.numero_de_atributos(); ++j) {
    if (codigo.niveles[indice(j)] < codigo.niveles.back()) {
      codigo.niveles[indice(j)] = codigo.niveles.back();
    }
  }
}

int grados_de_libertad(const std::string& cad) {
  return static_cast<int>(std::count(cad.begin(), cad.end(), '?'));
}

std::string combinar_patrones(const std::string& patron1, const std::string& patron2) {
  std::string combinado = patron1;
  for (std::size_t i = 0; i < patron1.size(); ++i) {
    if (patron1[i] == patron2[i]) {
      combinado[i] = patron1[i];
    } else if ((patron1[i] == '1' && patron2[i] == '?') ||
               (patron1[i] == '?' && patron2[i] == '1')) {
      combinado[i] = '1';
    } else if ((patron1[i] == '0' && patron2[i] == '?') ||
               (patron1[i] == '?' && patron2[i] == '0')) {
      combinado[i] = '0';
    } else {
      combinado[i] = '?';
    }
  }
  return combinado;
}

}  // namespace

// En el original esperaba una tecla tras los avisos de consistencia del análisis de patrones.
// Como librería no puede bloquearse esperando la entrada estándar: solo avisa.
void aviso_de_consistencia() {
  comun::progreso(Verbosidad::detalle)
      << "INSLV: aviso de consistencia en el análisis de patrones (ver la salida anterior)\n";
}

// Las diferencias entre dos patrones, a la depuración.
void pintar_diferencias(const std::string& patron1, const std::string& patron2) {
  std::ostream& salida = comun::progreso(Verbosidad::depuracion);
  for (std::size_t s = 0; s < patron1.size(); ++s) {
    if (patron1[s] == patron2[s]) {
      salida << '_';
    } else if (patron1[s] == '0' && patron2[s] == '1') {
      salida << '>';
    } else if (patron1[s] == '1' && patron2[s] == '0') {
      salida << '<';
    } else {
      salida << '$';
    }
  }
  salida << '\n';
}

// Determina si se mantiene que esp es una especificacion de gen.
bool es_especificacion(const std::string& esp, const std::string& gen) {
  for (std::size_t i = 0; i < esp.size(); ++i) {
    if (esp[i] == '1' && gen[i] == '0') {
      return false;
    }
  }
  return true;
}

std::string patron_de_regla(const reglas::ConjuntoDeReglas& reglas, int regla) {
  // Sin esa regla, un patrón vacío. En el original leía la regla -1 cuando el genético no
  // añadía ninguna en la primera vuelta (patrones vacío) y el programa reventaba (HALLAZGOS).
  if (regla < 0 || regla >= reglas.numero_de_reglas()) {
    return "";
  }
  return reglas.codigo(regla).etiquetas;
}

std::string patron_de_dos_reglas(const reglas::ConjuntoDeReglas& reglas1,
                                 const reglas::ConjuntoDeReglas& reglas2,
                                 const difuso::Variables& variables, int regla) {
  const std::string regla1 = etiquetas_efectivas(reglas1.codigo(regla), variables);
  const std::string regla2 = etiquetas_efectivas(reglas2.codigo(regla), variables);
  std::string patron = regla1;
  for (std::size_t i = 0; i < regla1.size(); ++i) {
    if (regla1[i] != regla2[i]) {
      patron[i] = '?';
    }
  }
  return patron;
}

void quitar_reglas(reglas::ConjuntoDeReglas& reglas1, reglas::ConjuntoDeReglas& reglas2,
                   reglas::ConjuntoDeReglas& reglas3, const std::vector<int>& reglas_a_eliminar) {
  for (const int regla : std::views::reverse(reglas_a_eliminar)) {
    reglas1.quitar(regla);
    reglas2.quitar(regla);
    reglas3.quitar(regla);
  }
}

void combinar_reglas(const difuso::Variables& variables, reglas::ConjuntoDeReglas& discriminantes,
                     reglas::ConjuntoDeReglas& descriptivas, reglas::ConjuntoDeReglas& patrones,
                     int nuevas) {
  std::vector<bool> eliminar(indice(discriminantes.numero_de_reglas()), false);

  for (int i = 0; i < nuevas; ++i) {
    if (eliminar[indice(i)]) {
      continue;
    }
    const std::string patron_i = patron_de_dos_reglas(descriptivas, discriminantes, variables, i);
    const std::string descriptiva_i = patron_de_regla(descriptivas, i);
    const std::string discriminante_i = patron_de_regla(discriminantes, i);
    const std::string patron_i_guardado = patrones.codigo(i).etiquetas;

    if (patron_i != patron_i_guardado) {
      comun::progreso(Verbosidad::depuracion)
          << "No coinciden los patrones para la regla " << i << '\n'
          << patron_i << '\n'
          << patron_i_guardado << '\n';
      aviso_de_consistencia();
    }

    if (discriminante_i != patron_i_guardado &&
        comparar_patrones(patron_i_guardado, discriminante_i) != 1) {
      comun::progreso(Verbosidad::depuracion)
          << "La regla discriminante " << i << " no coincide con su patron\n"
          << discriminante_i << '\n'
          << patron_i_guardado << '\n';
      aviso_de_consistencia();
    }

    if (descriptiva_i != patron_i_guardado &&
        comparar_patrones(patron_i_guardado, descriptiva_i) != 1) {
      comun::progreso(Verbosidad::depuracion)
          << "La regla descriptiva " << i << " no coincide con su patron\n"
          << descriptiva_i << '\n'
          << patron_i_guardado << '\n';
      aviso_de_consistencia();
    }

    for (int j = i + 1; j < discriminantes.numero_de_reglas() && !eliminar[indice(i)]; ++j) {
      if (eliminar[indice(j)]) {
        continue;
      }
      const std::string patron_j = patron_de_dos_reglas(descriptivas, discriminantes, variables, j);
      const int iguales = comparar_patrones(patron_j, patron_i_guardado);

      if (discriminantes.clase(i) != discriminantes.clase(j)) {
        if (iguales == 0) {
          comun::progreso(Verbosidad::detalle)
              << "las reglas " << i << " y " << j << " son contradictorias\n";
          discriminantes.sumar_fallos(i, discriminantes.aciertos(i));
          discriminantes.recalcular_peso(i);
          descriptivas.sumar_fallos(i, descriptivas.aciertos(i));
          descriptivas.recalcular_peso(i);
        } else if (iguales == 1) {
          comun::progreso(Verbosidad::detalle)
              << "las reglas " << i << " y " << j << " entran en conflicto\n";
        }
        continue;
      }

      // El caso raro de compatibilidades entre patrones
      if (iguales > 1 && comparar_patrones(descriptiva_i, patron_j) != 2 &&
          comparar_patrones(discriminante_i, patron_j) != 2) {
        comun::progreso(Verbosidad::depuracion)
            << "Caso especial de incompatibilidades entre patrones y si entre reglas\n"
            << descriptiva_i << '\n'
            << discriminante_i << '\n'
            << patron_i_guardado << "\n\n"
            << patron_de_regla(descriptivas, j) << '\n'
            << patron_de_regla(discriminantes, j) << '\n'
            << patron_j << "\n\n";
        aviso_de_consistencia();
      }

      if (iguales == 0) {
        // Los dos patrones son iguales
        eliminar[indice(i)] = true;
      } else if (iguales == 1) {
        // Los dos patrones son compatibles
        comun::progreso(Verbosidad::detalle) << "\t\t\t\t\tCombinando Reglas " << i << " y " << j
                                             << " de la clase " << discriminantes.clase(i) << '\n';

        intersecar_discriminantes(variables, discriminantes, j, discriminantes, i);
        unir_descriptivas(variables, descriptivas, j, descriptivas, i);

        const std::string resultado =
            patron_de_dos_reglas(descriptivas, discriminantes, variables, j);
        const std::string combinado = combinar_patrones(patron_i, patron_j);
        comun::progreso(Verbosidad::depuracion) << combinado << '\n';
        if (comparar_patrones(resultado, combinado) != 0) {
          comun::progreso(Verbosidad::depuracion) << resultado << '\n' << combinado << '\n';
          pintar_diferencias(resultado, combinado);
          aviso_de_consistencia();
        }

        patrones.codigo(j).etiquetas = combinado;
        std::string general = combinado;
        std::replace(general.begin(), general.end(), '?', '1');
        discriminantes.codigo(j).etiquetas = general;
        std::string especifica = combinado;
        std::replace(especifica.begin(), especifica.end(), '?', '0');
        descriptivas.codigo(j).etiquetas = especifica;

        if (patron_de_dos_reglas(descriptivas, discriminantes, variables, j) != combinado) {
          comun::progreso(Verbosidad::depuracion)
              << "Error al componer la descrip. y la predic. en la regla " << j << '\n';
          aviso_de_consistencia();
        }

        eliminar[indice(i)] = true;

        if (grados_de_libertad(patron_j) >= grados_de_libertad(patron_i_guardado)) {
          descriptivas.sumar_aciertos(j, descriptivas.aciertos(i));
          descriptivas.sumar_fallos(j, descriptivas.fallos(i));
          discriminantes.sumar_aciertos(j, discriminantes.aciertos(i));
          discriminantes.sumar_fallos(j, discriminantes.fallos(i));
        }
      }
    }
  }

  std::vector<int> eliminables;
  for (int i = 0; i < discriminantes.numero_de_reglas() - 1; ++i) {
    if (eliminar[indice(i)]) {
      eliminables.push_back(i);
    }
  }

  quitar_reglas(discriminantes, descriptivas, patrones, eliminables);
}

}  // namespace inslv::aprendizaje
