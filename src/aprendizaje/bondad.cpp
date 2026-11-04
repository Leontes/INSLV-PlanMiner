// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/aprendizaje/bondad.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

// Si la valoración v2 es mejor que la v en sus tres primeros valores.
bool Mejor(const Fitness& v2, const Fitness& v) {
  return (v2[0] > v[0]) || (v2[0] == v[0] && v2[1] > v[1]) ||
         (v2[0] == v[0] && v2[1] == v[1] && v2[2] > v[2]);
}

// Igual, o si es igual de buena.
bool MejorOIgual(const Fitness& v2, const Fitness& v) {
  return (v2[0] > v[0]) || (v2[0] == v[0] && v2[1] > v[1]) ||
         (v2[0] == v[0] && v2[1] == v[1] && v2[2] >= v[2]);
}

// Si s tiene algún cero: si no, el atributo admite todas sus etiquetas y es irrelevante.
bool Relevante(std::string_view s) {
  return !std::all_of(s.begin(), s.end(), [](char bit) { return bit == '1'; });
}

}  // namespace

void fitness(const Evaluacion& ev) {
  multipoblacion& G = ev.G;
  // Las reglas que eliminaría cada individuo solo interesan al evaluar la regla que se añade.
  std::vector<int> reglas_a_eliminar;
  Fitness valor{};
  for (int i = 0; i < G.N_individuos(); ++i) {
    if (G.Modificado(i)) {
      Bondad(ev, i, valor, reglas_a_eliminar);
      G.Valorar(i, valor);
    }
  }
}

void Obtener_Regla_Discriminante(const Evaluacion& ev, int elemento_i,
                                 std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  std::string& etiquetas = G.Etiquetas(elemento_i);
  std::vector<double>& niveles = G.Niveles(elemento_i);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  std::size_t tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto longitud = indice(V.Variable(static_cast<int>(j)).N_etiquetas());
    if (niveles[j] < umbral) {
      etiquetas.replace(tama, longitud, longitud, '1');
    }
    tama += longitud;
  }

  Fitness valor{};
  Fitness valor2{};
  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);

  // Quita cada atributo relevante si la regla mejora sin él.
  for (std::size_t j = 0; j < atributos; ++j) {
    if (niveles[j] >= umbral) {
      const double aux = niveles[j];
      niveles[j] = umbral - 0.001;

      Bondad(ev, elemento_i, valor2, reglas_a_eliminar);

      if (Mejor(valor2, valor)) {
        valor = valor2;
      } else {
        niveles[j] = aux;
      }
    }
  }

  // Admite cada etiqueta más si la regla no empeora.
  tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const double aux2 = niveles[j];
    const auto longitud = indice(V.Variable(static_cast<int>(j)).N_etiquetas());

    if (niveles[j] >= umbral) {
      for (std::size_t s = 0; s < longitud; ++s) {
        if (etiquetas[tama + s] == '0') {
          etiquetas[tama + s] = '1';
          Bondad(ev, elemento_i, valor2, reglas_a_eliminar);
          if (MejorOIgual(valor2, valor)) {
            valor = valor2;
          } else {
            etiquetas[tama + s] = '0';
            niveles[j] = aux2;
          }
        }
      }
    }
    tama += longitud;
  }

  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);
}

void Obtener_Regla_Descriptiva(const Evaluacion& ev, int elemento_i,
                               std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  std::string& etiquetas = G.Etiquetas(elemento_i);
  std::vector<double>& niveles = G.Niveles(elemento_i);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  Fitness valor{};
  Fitness valor2{};
  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);

  std::size_t tama = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto longitud = indice(V.Variable(static_cast<int>(j)).N_etiquetas());

    if (niveles[j] < umbral) {
      // Añade el atributo a la regla si, quitándole alguna etiqueta, no empeora.
      const double aux = niveles[j];
      bool mejora = false;
      niveles[j] = umbral;
      etiquetas.replace(tama, longitud, longitud, '1');

      for (std::size_t s = 0; s < longitud; ++s) {
        etiquetas[tama + s] = '0';
        Bondad(ev, elemento_i, valor2, reglas_a_eliminar);

        if (MejorOIgual(valor2, valor)) {
          valor = valor2;
          mejora = true;
        } else {
          etiquetas[tama + s] = '1';
          niveles[j] = umbral;
        }
      }

      if (!mejora) {
        niveles[j] = aux;
      }
    } else {
      // Quita cada etiqueta si la regla mejora sin ella.
      for (std::size_t s = 0; s < longitud; ++s) {
        if (etiquetas[tama + s] == '1') {
          etiquetas[tama + s] = '0';
          Bondad(ev, elemento_i, valor2, reglas_a_eliminar);
          if (Mejor(valor2, valor)) {
            valor = valor2;
          } else {
            etiquetas[tama + s] = '1';
          }
        }
      }
    }

    tama += longitud;
  }

  Bondad(ev, elemento_i, valor, reglas_a_eliminar);
  G.Valorar(elemento_i, valor);
}

double Weight_Individual(const multipoblacion& G, const example_set& E,
                         const TablaDeAdaptaciones& tabla, int elemento_i, double& n_mas,
                         double& n_menos) {
  const std::string& etiquetas = G.Etiquetas(elemento_i);
  const std::vector<double>& niveles = G.Niveles(elemento_i);
  const int clase = G.ClaseIndividuo(elemento_i);

  double positivos = 0;
  double negativos = 0;
  for (int j = 0; j < E.N_Examples(); ++j) {
    double aux_p = 0;
    double aux_n = 0;
    tabla.Consecuente(j, clase, aux_p, aux_n);
    const double x = tabla.Antecedente(j, etiquetas, niveles);
    positivos = positivos + (x * aux_p);
    negativos = negativos + (x * aux_n);
  }

  n_mas = positivos;
  n_menos = negativos;

  return (positivos + 1) / (positivos + negativos + 1);
}

double Bondad(const Evaluacion& ev, int elemento_i, Fitness& valor,
              std::vector<int>& reglas_a_eliminar) {
  multipoblacion& G = ev.G;
  const VectorVar& V = ev.V;
  const example_set& E = ev.E;
  const ruleset& R = ev.R;
  const std::vector<int>& ponderacion_por_clase = ev.ponderacion_por_clase;

  const int ne = E.N_Examples();
  const int nrule = R.N_rule();
  const int clase = G.ClaseIndividuo(elemento_i);
  const int ponderacion = ponderacion_por_clase[indice(clase)];

  const std::string& etiquetas = G.Etiquetas(elemento_i);
  std::vector<double>& niveles = G.Niveles(elemento_i);
  const std::size_t atributos = niveles.size() - 1;
  const double umbral = niveles.back();

  std::size_t variables_irrelevantes = 0;
  int ceros_variables = 0;
  std::size_t pos = 0;  // posicion en el cromosoma nivel de valor

  for (std::size_t j = 0; j < atributos; ++j) {
    const auto tamanio = indice(V.Variable(static_cast<int>(j)).N_etiquetas());
    const std::string_view trozo = std::string_view(etiquetas).substr(pos, tamanio);
    if (niveles[j] < umbral) {
      ++variables_irrelevantes;
    } else if (Relevante(trozo)) {
      ceros_variables += static_cast<int>(std::count(trozo.begin(), trozo.end(), '0'));
    } else {
      niveles[j] = umbral - 0.001;
      ++variables_irrelevantes;
    }
    pos = pos + tamanio;
  }

  const int variables_relevantes = static_cast<int>(atributos - variables_irrelevantes);

  // Comprobar que es una regla valida
  if (!V.Es_Valida(etiquetas, niveles) || variables_irrelevantes >= atributos) {
    valor.fill(-2 * ne);
    return 0;
  }

  // Calculo previo para obtener el peso de la regla
  std::vector<double> adaptacion(indice(ne));
  double positivos = 0;
  double negativos = 0;
  double afecta = 0;
  for (int j = 0; j < ne; ++j) {
    double aux_p = 0;
    double aux_n = 0;
    ev.tabla.Consecuente(j, clase, aux_p, aux_n);

    const double a = ev.tabla.Antecedente(j, etiquetas, niveles);

    if (a * aux_p > 0) {
      ++afecta;
    }

    positivos = positivos + (a * aux_p);
    negativos = negativos + (a * aux_n);
    adaptacion[indice(j)] = aux_p < aux_n ? -a : a;
  }

  const double peso = (positivos + 1) / (positivos + negativos + 1);
  const bool menos_infinito = !(positivos > 0);

  if (ponderacion > 0) {
    afecta = afecta / ponderacion;
  }

  // Dado el peso de la regla obtener la bondad de la misma.
  std::vector<int> basica_esta_regla(indice(nrule));
  std::vector<int> errores_esta_regla(indice(nrule));
  const auto anota = [&](std::vector<int>& cuenta, int ejemplo, int aislada) {
    if (E.Rule_Aislada(ejemplo) == aislada && E.Rule_Used(ejemplo) >= 0 &&
        E.Rule_Used(ejemplo) < nrule) {
      ++cuenta[indice(E.Rule_Used(ejemplo))];
    }
  };
  double aciertos = 0;
  double fallos = 0;
  positivos = 0;
  negativos = 0;
  for (int j = 0; j < ne; ++j) {
    const double a = adaptacion[indice(j)];
    if (a > 0) {
      // La regla gana a la mejor de otra clase.
      if (a * peso > E.Grade_Is_Negative_Covered(j) ||
          (a * peso == E.Grade_Is_Negative_Covered(j) && peso > E.Peso_Is_Negative_Covered(j))) {
        if (!E.Is_Covered(j)) {
          positivos = positivos + (a * peso);
          ++aciertos;
          anota(errores_esta_regla, j, -1);
        } else {
          anota(basica_esta_regla, j, 1);
        }
      }
    } else if (a < 0) {
      // La regla gana a la mejor de la clase del ejemplo.
      if (-a * peso > E.Grade_Is_Positive_Covered(j) ||
          (-a * peso == E.Grade_Is_Positive_Covered(j) && peso > E.Peso_Is_Positive_Covered(j))) {
        negativos =
            negativos +
            ((-a * peso) / ponderacion_por_clase[indice(E.Class_of_example(V.Consecuente(), j))]);
        if (!E.Is_Covered(j)) {
          anota(errores_esta_regla, j, -1);
        } else {
          ++fallos;
          anota(basica_esta_regla, j, 1);
        }
      }
    }
  }

  // Evaluar si es posible eliminar alguna regla ya existente
  int reglas_eliminables = 0;
  double aciertos_adicionales = 0;
  int ganancia_en_variables = variables_relevantes;
  double diferencia_peso = peso;

  reglas_a_eliminar.clear();
  for (int i = ev.tam_rule_not_removed; i < nrule; ++i) {
    const std::size_t r = indice(i);
    const int ganancia = (basica_esta_regla[r] - ev.basica_para_regla[r]) +
                         (ev.errores_para_regla[r] - errores_esta_regla[r]);
    if (ganancia >= 0) {
      reglas_a_eliminar.push_back(i);
      ++reglas_eliminables;
      aciertos_adicionales += ganancia;
      ganancia_en_variables = R.Variables_of_rule(i) - ganancia_en_variables;
      diferencia_peso -= R.Get_Weight(i);
    }
  }

  positivos = ponderacion > 0 ? positivos / ponderacion : 0;

  if (aciertos + aciertos_adicionales - fallos > 0) {
    if (ponderacion > 0) {
      if (positivos - negativos < 0) {
        valor[0] = -0.9;
      } else {
        valor[0] = positivos - negativos;
        valor[0] *= peso * peso * peso * peso * peso * peso * peso * peso * peso;
      }
      if (positivos == 0 && menos_infinito) {
        valor[0] = -2 * ne;
      }
    } else {
      valor[0] = -2 * ne;
    }
  } else if (aciertos + aciertos_adicionales > 0) {
    valor[0] = aciertos + aciertos_adicionales - fallos;
    if (valor[0] == 0) {
      valor[0] = -ne;
    } else if (ev.reducir_reglas && !(reglas_eliminables >= (-valor[0]) + 2)) {
      valor[0] = -2 * ne;
    }
  } else {
    valor[0] = -(ne + fallos);
  }

  valor[6] = aciertos_adicionales;
  valor[5] = aciertos - fallos;
  valor[4] = 0;
  valor[3] = -variables_relevantes;
  valor[2] = afecta;
  valor[1] = reglas_eliminables;

  if (reglas_eliminables == 1 && (valor[0] == -ne || valor[0] == 0)) {
    valor[2] = aciertos_adicionales > 0 ? 0.1 + aciertos_adicionales : diferencia_peso;
    valor[3] = -ganancia_en_variables;
    valor[4] = 0;
    valor[5] = ceros_variables;
    // En el original, con una sola regla leía el peso de una regla que no existe: memoria sin
    // inicializar o de una regla ya quitada. No decide nada: dos individuos solo empatan hasta
    // aquí si los dos pasan por esta rama, y entonces leen el mismo valor (y forzándolo a 0, 1e9 y
    // -1e9, las reglas de los conjuntos de prueba no cambian).
    valor[6] = reglas_eliminables < nrule ? -R.Get_Weight(reglas_eliminables) : 0;
  }

  return peso;
}

}  // namespace inslv
