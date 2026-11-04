// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/busqueda.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "inslv/aprendizaje/patrones.hpp"
#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/azar.hpp"

namespace inslv::aprendizaje {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Un individuo por cada elemento de la población: un ejemplo al azar de su clase sin clasificar
// bien, como la mejor etiqueta de cada atributo; -1 si no quedan ejemplos de la clase.
std::vector<std::vector<int>> sujetos_iniciales(const difuso::Variables& variables,
                                                const datos::Ejemplos& ejemplos, int clases,
                                                int n_individuos, const TablaDeAdaptaciones& tabla,
                                                const std::vector<int>& n_ejemplos_por_clase) {
  std::vector<std::vector<int>> matriz(
      indice(n_individuos), std::vector<int>(indice(variables.numero_de_atributos()), -1));
  for (int i = 0; i < n_individuos; ++i) {
    if (n_ejemplos_por_clase[indice(i % clases)] != 0) {
      int a = genetico::individuo_al_azar(ejemplos.numero_de_ejemplos(), -1, 0);
      while (ejemplos.bien_clasificado(a) ||
             tabla.adaptacion_a_etiqueta(a, variables.consecuente(), i % clases) == 0) {
        a = (a + 1) % ejemplos.numero_de_ejemplos();
      }
      for (int j = 0; j < variables.numero_de_atributos(); ++j) {
        matriz[indice(i)][indice(j)] = tabla.mejor_etiqueta(a, j);
      }
    }
  }
  return matriz;
}

}  // namespace

void buscar_regla(const Evaluacion& evaluacion, reglas::ConjuntoDeReglas& descriptivas,
                  reglas::ConjuntoDeReglas& discriminantes, reglas::ConjuntoDeReglas& patrones,
                  const std::vector<std::vector<double>>& medidas, bool primera_vez,
                  std::vector<int>& reglas_a_eliminar, bool& anadida) {
  genetico::MultiPoblacion& poblacion = evaluacion.poblacion;
  const difuso::Variables& variables = evaluacion.variables;
  const datos::Ejemplos& ejemplos = evaluacion.ejemplos;

  anadida = false;

  const int n_clases = variables.numero_de_etiquetas(variables.consecuente());

  if (descriptivas.numero_de_reglas() == 0 || primera_vez) {
    const std::vector<int> n_ejemplos_por_clase =
        ejemplos.sin_clasificar_bien_por_clase(variables.consecuente(), n_clases);
    const std::vector<std::vector<int>> sujetos =
        sujetos_iniciales(variables, ejemplos, n_clases, poblacion.numero_de_individuos(),
                          evaluacion.tabla, n_ejemplos_por_clase);
    std::vector<int> etiquetas_por_atributo;
    etiquetas_por_atributo.reserve(static_cast<std::size_t>(variables.numero_de_atributos()));
    for (int i = 0; i < variables.numero_de_atributos(); ++i) {
      etiquetas_por_atributo.push_back(variables.numero_de_etiquetas(i));
    }
    poblacion.inicializar(medidas, sujetos, etiquetas_por_atributo);
  } else {
    for (int i = 0; i < poblacion.numero_de_individuos(); ++i) {
      poblacion.marcar_modificado(i);
    }
  }

  valorar_modificados(evaluacion);

  for (int i = poblacion.numero_de_individuos() - 2; i < poblacion.numero_de_individuos(); ++i) {
    especializar(evaluacion, i, reglas_a_eliminar);
  }

  poblacion.ordenar();

  genetico::Valoracion mejor_anterior = poblacion.valoracion(0);

  const int max_iter = 500;
  int sin_cambios = 0;
  while (sin_cambios <= max_iter) {
    if ((descriptivas.numero_de_reglas() > 1 && poblacion.valoracion(0)[0] <= 0 &&
         genetico::con_probabilidad(0.75)) ||
        (descriptivas.numero_de_reglas() > 1 && genetico::con_probabilidad(0.01))) {
      // En el original elegía aquí una regla que luego no usaba. Se mantiene por el número que
      // pide.
      static_cast<void>(genetico::individuo_al_azar(descriptivas.numero_de_reglas(), -1, -1));
      poblacion.mutar_niveles();
    } else {
      poblacion.cruzar((1.0 * sin_cambios) / max_iter);
      poblacion.mutar();
    }
    valorar_modificados(evaluacion);
    poblacion.colocar_los_dos_ultimos();

    if (poblacion.valoracion(0) == mejor_anterior) {
      ++sin_cambios;
      if (sin_cambios == 2) {
        const int segundo = 1;
        poblacion.poner_codigo(segundo, poblacion.codigo(0));

        if (poblacion.valoracion(segundo)[0] != -ejemplos.numero_de_ejemplos() ||
            poblacion.valoracion(segundo)[1] != 1) {
          generalizar(evaluacion, segundo, reglas_a_eliminar);
        } else {
          especializar(evaluacion, segundo, reglas_a_eliminar);
        }
      }
    } else {
      sin_cambios = 0;
      mejor_anterior = poblacion.valoracion(0);
    }
  }

  const genetico::Valoracion& mejor = poblacion.valoracion(0);
  const double menos_ne = -ejemplos.numero_de_ejemplos();

  if (mejor[0] <= -1 && mejor[1] > 1 && mejor[1] >= (-mejor[0]) + 2) {
    comun::progreso(Verbosidad::depuracion) << "->->->->->-> El nuevo caso especial\n";
  }

  if ((mejor[0] > -1) || (mejor[0] <= -1 && mejor[1] > 1 && mejor[1] >= (-mejor[0]) + 2) ||
      (mejor[0] == menos_ne && mejor[1] > 1) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] > 0) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] == 0 && mejor[3] > 0) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] == 0 && mejor[3] == 0 && mejor[4] > 0)) {
    genetico::Codigo discriminante;
    genetico::Codigo descriptiva;
    genetico::Valoracion valor{};
    std::array<double, 2> esp{};
    std::array<double, 2> gen{};
    double peso = 0;
    double aciertos_discriminante = 0;
    double aciertos_descriptiva = 0;
    double fallos_discriminante = 0;
    double fallos_descriptiva = 0;

    do {
      generalizar(evaluacion, 0, reglas_a_eliminar);
      discriminante = poblacion.codigo(0);
      bondad(evaluacion, 0, valor, reglas_a_eliminar);
      peso_del_individuo(poblacion, ejemplos, evaluacion.tabla, 0, aciertos_discriminante,
                         fallos_discriminante);
      gen = {valor[0], valor[1]};

      especializar(evaluacion, 0, reglas_a_eliminar);
      bondad(evaluacion, 0, valor, reglas_a_eliminar);
      peso = peso_del_individuo(poblacion, ejemplos, evaluacion.tabla, 0, aciertos_descriptiva,
                                fallos_descriptiva);
      esp = {valor[0], valor[1]};
      descriptiva = poblacion.codigo(0);
    } while (esp != gen || !es_especificacion(descriptiva.etiquetas, discriminante.etiquetas));

    descriptivas.anadir(descriptiva, aciertos_descriptiva, fallos_descriptiva, false);
    discriminantes.anadir(discriminante, aciertos_discriminante, fallos_discriminante, false);

    const std::string patron = patron_de_dos_reglas(descriptivas, discriminantes, variables,
                                                    descriptivas.numero_de_reglas() - 1);

    const std::string especifica =
        patron_de_regla(descriptivas, descriptivas.numero_de_reglas() - 1);
    const std::string general =
        patron_de_regla(discriminantes, discriminantes.numero_de_reglas() - 1);
    if (!es_especificacion(especifica, general)) {
      comun::progreso(Verbosidad::depuracion) << "esp: \n"
                                              << especifica << '\n'
                                              << general << "\n:gen\n";
      pintar_diferencias(especifica, general);
      aviso_de_consistencia();
    }

    discriminante.etiquetas = patron;
    patrones.anadir(discriminante, peso);

    anadida = true;

    // Pongo la regla por defecto para la clase obtenida en la posición penultima de la poblacion
    discriminante.etiquetas.assign(discriminante.etiquetas.size(), '1');

    poblacion.poner_codigo(0, descriptiva);

    // Pongo la regla por defecto para otra clase en la ultima posicion de la poblacion
    discriminante.clase = (discriminante.clase + 1) % n_clases;
    poblacion.poner_codigo(poblacion.numero_de_individuos() - 1, discriminante);
    discriminante.clase = (discriminante.clase + 1) % n_clases;
    poblacion.poner_codigo(poblacion.numero_de_individuos() - 2, discriminante);
  }
}

}  // namespace inslv::aprendizaje
