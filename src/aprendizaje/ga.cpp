// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/ga.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "inslv/aprendizaje/patrones.h"
#include "inslv/comun/progreso.hpp"
#include "inslv/genetico/azar.hpp"

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Un individuo por cada elemento de la población: un ejemplo al azar de su clase sin clasificar
// bien, como la mejor etiqueta de cada atributo; -1 si no quedan ejemplos de la clase.
std::vector<std::vector<int>> Encontrar_Individuos_dados_ejemplos(
    const difuso::Variables& V, const datos::Ejemplos& E, int nclases, int n_individuos,
    const TablaDeAdaptaciones& tabla, const std::vector<int>& n_ejemplos_por_clase) {
  std::vector<std::vector<int>> matriz(indice(n_individuos),
                                       std::vector<int>(indice(V.numero_de_atributos()), -1));
  for (int i = 0; i < n_individuos; ++i) {
    if (n_ejemplos_por_clase[indice(i % nclases)] != 0) {
      int a = genetico::individuo_al_azar(E.numero_de_ejemplos(), -1, 0);
      while (E.bien_clasificado(a) || tabla.Etiqueta(a, V.consecuente(), i % nclases) == 0) {
        a = (a + 1) % E.numero_de_ejemplos();
      }
      for (int j = 0; j < V.numero_de_atributos(); ++j) {
        matriz[indice(i)][indice(j)] = tabla.MejorEtiqueta(a, j);
      }
    }
  }
  return matriz;
}

}  // namespace

void GA(const Evaluacion& ev, ruleset& R, ruleset& R_dis, ruleset& R_mix,
        const std::vector<std::vector<double>>& I, bool primeraVez,
        std::vector<int>& reglas_a_eliminar, bool& Added_Rule) {
  genetico::MultiPoblacion& G = ev.G;
  const difuso::Variables& V = ev.V;
  const datos::Ejemplos& E = ev.E;

  Added_Rule = false;

  const int n_clases = V.numero_de_etiquetas(V.consecuente());

  if (R.N_rule() == 0 || primeraVez) {
    const std::vector<int> n_ejemplos_por_clase =
        E.sin_clasificar_bien_por_clase(V.consecuente(), n_clases);
    const std::vector<std::vector<int>> sujetos = Encontrar_Individuos_dados_ejemplos(
        V, E, n_clases, G.numero_de_individuos(), ev.tabla, n_ejemplos_por_clase);
    std::vector<int> lista_tamano;
    lista_tamano.reserve(static_cast<std::size_t>(V.numero_de_atributos()));
    for (int i = 0; i < V.numero_de_atributos(); ++i) {
      lista_tamano.push_back(V.numero_de_etiquetas(i));
    }
    G.inicializar(I, sujetos, lista_tamano);
  } else {
    for (int i = 0; i < G.numero_de_individuos(); ++i) {
      G.marcar_modificado(i);
    }
  }

  fitness(ev);

  for (int ik = G.numero_de_individuos() - 2; ik < G.numero_de_individuos(); ++ik) {
    Obtener_Regla_Descriptiva(ev, ik, reglas_a_eliminar);
  }

  G.ordenar();

  genetico::Valoracion mejor_anterior = G.valoracion(0);

  const int max_iter = 500;
  int sin_cambios = 0;
  while (sin_cambios <= max_iter) {
    if ((R.N_rule() > 1 && G.valoracion(0)[0] <= 0 && genetico::con_probabilidad(0.75)) ||
        (R.N_rule() > 1 && genetico::con_probabilidad(0.01))) {
      // En el original elegía aquí una regla que luego no usaba. Se mantiene por el número que
      // pide.
      static_cast<void>(genetico::individuo_al_azar(R.N_rule(), -1, -1));
      G.mutar_niveles();
    } else {
      G.cruzar((1.0 * sin_cambios) / max_iter);
      G.mutar();
    }
    fitness(ev);
    G.colocar_los_dos_ultimos();

    if (G.valoracion(0) == mejor_anterior) {
      ++sin_cambios;
      if (sin_cambios == 2) {
        const int t = 1;
        G.poner_codigo(t, G.codigo(0));

        if (G.valoracion(t)[0] != -E.numero_de_ejemplos() || G.valoracion(t)[1] != 1) {
          Obtener_Regla_Discriminante(ev, t, reglas_a_eliminar);
        } else {
          Obtener_Regla_Descriptiva(ev, t, reglas_a_eliminar);
        }
      }
    } else {
      sin_cambios = 0;
      mejor_anterior = G.valoracion(0);
    }
  }

  const genetico::Valoracion& mejor = G.valoracion(0);
  const double menos_ne = -E.numero_de_ejemplos();

  if (mejor[0] <= -1 && mejor[1] > 1 && mejor[1] >= (-mejor[0]) + 2) {
    comun::progreso(Verbosidad::depuracion) << "->->->->->-> El nuevo caso especial\n";
  }

  if ((mejor[0] > -1) || (mejor[0] <= -1 && mejor[1] > 1 && mejor[1] >= (-mejor[0]) + 2) ||
      (mejor[0] == menos_ne && mejor[1] > 1) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] > 0) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] == 0 && mejor[3] > 0) ||
      (mejor[0] == menos_ne && mejor[1] == 1 && mejor[2] == 0 && mejor[3] == 0 && mejor[4] > 0)) {
    genetico::Codigo code1;
    genetico::Codigo code2;
    genetico::Valoracion valor{};
    std::array<double, 2> esp{};
    std::array<double, 2> gen{};
    double peso = 0;
    double nmas1 = 0;
    double nmas2 = 0;
    double nmenos1 = 0;
    double nmenos2 = 0;

    do {
      Obtener_Regla_Discriminante(ev, 0, reglas_a_eliminar);
      code1 = G.codigo(0);
      Bondad(ev, 0, valor, reglas_a_eliminar);
      Weight_Individual(G, E, ev.tabla, 0, nmas1, nmenos1);
      gen = {valor[0], valor[1]};

      Obtener_Regla_Descriptiva(ev, 0, reglas_a_eliminar);
      Bondad(ev, 0, valor, reglas_a_eliminar);
      peso = Weight_Individual(G, E, ev.tabla, 0, nmas2, nmenos2);
      esp = {valor[0], valor[1]};
      code2 = G.codigo(0);
    } while (esp != gen || !ComprobacionEspGen(code2.etiquetas, code1.etiquetas));

    R.Add(code2, nmas2, nmenos2, false);
    R_dis.Add(code1, nmas1, nmenos1, false);

    const std::string patron = Patron2Reglas(R, R_dis, V, R.N_rule() - 1);

    const std::string especifica = Patron1Regla(R, R.N_rule() - 1);
    const std::string general = Patron1Regla(R_dis, R_dis.N_rule() - 1);
    if (!ComprobacionEspGen(especifica, general)) {
      comun::progreso(Verbosidad::depuracion) << "esp: \n"
                                              << especifica << '\n'
                                              << general << "\n:gen\n";
      Comparar2patronesPinta(especifica, general);
      Pausa();
    }

    code1.etiquetas = patron;
    R_mix.Add(code1, peso);

    Added_Rule = true;

    // Pongo la regla por defecto para la clase obtenida en la posición penultima de la poblacion
    code1.etiquetas.assign(code1.etiquetas.size(), '1');

    G.poner_codigo(0, code2);

    // Pongo la regla por defecto para otra clase en la ultima posicion de la poblacion
    code1.clase = (code1.clase + 1) % n_clases;
    G.poner_codigo(G.numero_de_individuos() - 1, code1);
    code1.clase = (code1.clase + 1) % n_clases;
    G.poner_codigo(G.numero_de_individuos() - 2, code1);
  }
}

}  // namespace inslv
