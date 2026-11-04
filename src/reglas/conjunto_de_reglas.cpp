// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          conjunto_de_reglas.cpp  -  description
                             -------------------
    begin                : Mon May 6 2002
    copyright            : (C) 2002 by Raúl Pérez
    email                : fgr@decsai.ugr.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "inslv/reglas/conjunto_de_reglas.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "inslv/comun/progreso.hpp"

namespace inslv::reglas {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

double peso_de(double mas, double menos) { return (mas + 1) / (mas + menos + 1); }

// Los atributos con nivel por encima del umbral.
int sobre_el_umbral(const genetico::Codigo& codigo) {
  const std::vector<double>& niveles = codigo.niveles;
  return static_cast<int>(std::count_if(niveles.begin(), niveles.end() - 1,
                                        [&](double nivel) { return nivel >= niveles.back(); }));
}

}  // namespace

void ConjuntoDeReglas::comprobar(int i) const {
  // En el original no se comprobaba: leía otra memoria o un puntero sin inicializar.
  if (i < 0 || i >= numero_de_reglas()) {
    throw std::logic_error("inslv: no existe la regla " + std::to_string(i));
  }
}

const ConjuntoDeReglas::ReglaCodificada& ConjuntoDeReglas::regla_i(int i) const {
  comprobar(i);
  return reglas_[indice(i)];
}

ConjuntoDeReglas::ReglaCodificada& ConjuntoDeReglas::regla_i(int i) {
  comprobar(i);
  return reglas_[indice(i)];
}

void ConjuntoDeReglas::poner_variables(const difuso::Variables& x) { variables_ = x; }

void ConjuntoDeReglas::anadir(const genetico::Codigo& x, double peso) {
  reglas_.push_back({x, peso, 0, 0, true});
}

void ConjuntoDeReglas::anadir(const genetico::Codigo& x, double mas, double menos, bool modif) {
  reglas_.push_back({x, peso_de(mas, menos), mas, menos, modif});
}

void ConjuntoDeReglas::anadir_al_principio(const ConjuntoDeReglas& r) {
  std::vector<ReglaCodificada> todas = r.reglas_;
  // Así en el original (modificada[i] = r.peso[i]), seguramente por error: modificada queda cierta
  // salvo con peso 0. Cambiarlo cambia las reglas.
  for (ReglaCodificada& regla : todas) {
    regla.modificada = regla.peso != 0;
  }
  todas.insert(todas.end(), std::make_move_iterator(reglas_.begin()),
               std::make_move_iterator(reglas_.end()));
  reglas_ = std::move(todas);
}

void ConjuntoDeReglas::quitar_ultima() { reglas_.pop_back(); }

void ConjuntoDeReglas::quitar(int i) {
  if (i >= 0 && i < numero_de_reglas()) {
    reglas_.erase(reglas_.begin() + i);
  }
}

genetico::Codigo& ConjuntoDeReglas::codigo(int i) { return regla_i(i).codigo; }

const genetico::Codigo& ConjuntoDeReglas::codigo(int i) const { return regla_i(i).codigo; }

void ConjuntoDeReglas::sumar_ejemplos(int i, double mas, double menos) {
  ReglaCodificada& regla = regla_i(i);
  regla.aciertos += mas;
  regla.fallos += menos;
  regla.peso = peso_de(regla.aciertos, regla.fallos);
}

double ConjuntoDeReglas::peso(int i) const { return regla_i(i).peso; }

void ConjuntoDeReglas::recalcular_peso(int i) {
  ReglaCodificada& regla = regla_i(i);
  regla.peso = peso_de(regla.aciertos, regla.fallos);
}

void ConjuntoDeReglas::recalcular_peso() {
  for (ReglaCodificada& regla : reglas_) {
    regla.peso = peso_de(regla.aciertos, regla.fallos);
  }
}

void ConjuntoDeReglas::sumar_aciertos(int i, double value) { regla_i(i).aciertos += value; }

void ConjuntoDeReglas::sumar_fallos(int i, double value) { regla_i(i).fallos += value; }

double ConjuntoDeReglas::aciertos(int i) const { return regla_i(i).aciertos; }

double ConjuntoDeReglas::fallos(int i) const { return regla_i(i).fallos; }

void ConjuntoDeReglas::marcar_modificada(int i, bool value) { regla_i(i).modificada = value; }

bool ConjuntoDeReglas::modificada(int i) const { return regla_i(i).modificada; }

void ConjuntoDeReglas::marcar_todas_modificadas() {
  for (ReglaCodificada& regla : reglas_) {
    regla.modificada = true;
  }
}

int ConjuntoDeReglas::numero_de_reglas() const { return static_cast<int>(reglas_.size()); }

int ConjuntoDeReglas::numero_con_peso(double minimo) const {
  return static_cast<int>(std::count_if(reglas_.begin(), reglas_.end(),
                                        [&](const auto& regla) { return regla.peso >= minimo; }));
}

double ConjuntoDeReglas::atributos_por_regla() const {
  int total = 0;
  for (const ReglaCodificada& regla : reglas_) {
    total += sobre_el_umbral(regla.codigo);
  }
  return 1.0 * total / numero_de_reglas();
}

int ConjuntoDeReglas::atributos_sobre_el_umbral(int i) const {
  return sobre_el_umbral(regla_i(i).codigo);
}

int ConjuntoDeReglas::etiquetas_excluidas(int i) const {
  const genetico::Codigo& codigo = regla_i(i).codigo;
  const std::size_t atributos = codigo.niveles.size() - 1;
  int ceros = 0;
  std::size_t donde = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto n_casos = indice(variables_.numero_de_etiquetas(static_cast<int>(j)));
    if (codigo.niveles[j] >= codigo.niveles.back()) {
      const std::string_view etiquetas = std::string_view(codigo.etiquetas).substr(donde, n_casos);
      ceros += static_cast<int>(std::count(etiquetas.begin(), etiquetas.end(), '0'));
    }
    donde += n_casos;
  }
  return ceros;
}

std::vector<ConjuntoDeReglas::AtributoRelevante> ConjuntoDeReglas::relevantes(
    const genetico::Codigo& codigo) const {
  std::vector<AtributoRelevante> relevantes;
  std::size_t donde = 0;
  for (int j = 0; j < variables_.numero_de_atributos(); ++j) {
    const auto n_casos = indice(variables_.numero_de_etiquetas(j));
    const std::string_view etiquetas = std::string_view(codigo.etiquetas).substr(donde, n_casos);
    if (codigo.niveles[indice(j)] >= codigo.niveles.back() &&
        !std::all_of(etiquetas.begin(), etiquetas.end(), [](char bit) { return bit == '1'; })) {
      relevantes.push_back({j, donde, n_casos});
    }
    donde += n_casos;
  }
  return relevantes;
}

double ConjuntoDeReglas::frecuencia_de_atributos(std::vector<int>& frec) const {
  // En el original leía la regla 0 para saber cuántos atributos hay, aunque no hubiera reglas.
  frec.assign(indice(variables_.numero_de_atributos()), 0);
  for (const ReglaCodificada& regla : reglas_) {
    for (const AtributoRelevante& relevante : relevantes(regla.codigo)) {
      ++frec[indice(relevante.atributo)];
    }
  }
  const auto usados = std::count_if(frec.begin(), frec.end(), [](int veces) { return veces > 0; });
  return static_cast<double>(usados) / static_cast<double>(frec.size());
}

int ConjuntoDeReglas::numero_de_condiciones() const {
  std::size_t condiciones = 0;
  for (const ReglaCodificada& regla : reglas_) {
    condiciones += relevantes(regla.codigo).size();
  }
  return static_cast<int>(condiciones);
}

double ConjuntoDeReglas::adaptacion_por_peso(const datos::Ejemplo& v,
                                             const ReglaCodificada& regla) const {
  return variables_.adaptacion(v, regla.codigo.etiquetas, regla.codigo.niveles) * regla.peso;
}

int ConjuntoDeReglas::inferir(const datos::Ejemplo& v, double& grado, int& regla_disparada) const {
  double max = 0;
  int re = -1;
  for (int i = 0; i < numero_de_reglas(); ++i) {
    const ReglaCodificada& regla = regla_i(i);
    const double aux = adaptacion_por_peso(v, regla);
    if (aux > max || (re != -1 && aux > 0 && aux == max && regla.peso > regla_i(re).peso)) {
      max = aux;
      re = i;
      grado = aux;
    }
  }
  regla_disparada = re;
  return re != -1 ? clase(re) : -1;
}

int ConjuntoDeReglas::inferir(const datos::Ejemplo& v, int clase_del_ejemplo, double& grado_p,
                              double& peso_p, double& grado_n, double& peso_n, int& regla_disparada,
                              int& aislada) const {
  const int n_reglas = numero_de_reglas();
  // Lo que se adapta a cada regla: en positivo, las de su clase; en negativo, las de otra.
  std::vector<double> adaptacion_particular;
  adaptacion_particular.reserve(reglas_.size());
  double max = 0;
  int re = -1;
  bool clasifica_bien = false;
  int indice_pos = n_reglas;
  int indice_neg = n_reglas;
  grado_p = 0;
  peso_p = 0;
  grado_n = 0;
  peso_n = 0;

  for (int i = 0; i < n_reglas; ++i) {
    const ReglaCodificada& regla = regla_i(i);
    const double aux = adaptacion_por_peso(v, regla);
    const bool de_su_clase = clase(i) == clase_del_ejemplo;
    adaptacion_particular.push_back(de_su_clase ? aux : -aux);
    if (aux > 0) {
      if (de_su_clase) {
        if (aux > grado_p || (aux == grado_p && regla.peso > peso_p)) {
          grado_p = aux;
          peso_p = regla.peso;
          indice_pos = i;
        }
      } else if (aux > grado_n || (aux == grado_n && regla.peso > peso_n)) {
        grado_n = aux;
        peso_n = regla.peso;
        indice_neg = i;
      }
    }
    if (aux > max || (re != -1 && aux > 0 && aux == max && regla.peso > regla_i(re).peso)) {
      max = aux;
      re = i;
      clasifica_bien = de_su_clase;
    }
  }

  // Si la regla i, con esa adaptación, gana a la mejor de la otra parte.
  const auto gana = [&](int i, double adaptacion, double grado, double peso, int mejor) {
    const double peso_i = regla_i(i).peso;
    return adaptacion > grado || (adaptacion == grado && peso_i > peso) ||
           (adaptacion == grado && peso_i == peso && i < mejor);
  };

  if (clasifica_bien) {
    aislada = 0;
    for (int i = 0; i < n_reglas; ++i) {
      const double aux = adaptacion_particular[indice(i)];
      if (aux > 0 && gana(i, aux, grado_n, peso_n, indice_neg)) {
        ++aislada;
      }
    }
    if (aislada == 0) {
      std::ostream& salida = comun::progreso(Verbosidad::depuracion);
      salida << "Error!!!!  Clase " << clase_del_ejemplo << "  Grado Pos: " << grado_p
             << " Grado Neg: " << grado_n << "  Regla: " << re << '\n';
      for (const double aux : adaptacion_particular) {
        salida << '\t' << aux;
      }
      salida << '\n';
    }
  } else if (grado_p == 0) {
    aislada = -1000;
  } else {
    aislada = 0;
    for (int i = 0; i < n_reglas; ++i) {
      const double aux = adaptacion_particular[indice(i)];
      if (aux < 0 && gana(i, -aux, grado_p, peso_p, indice_pos)) {
        --aislada;
      }
    }
  }

  regla_disparada = re;
  return re != -1 ? clase(re) : -1;
}

int ConjuntoDeReglas::clase(int i) const { return regla_i(i).codigo.clase; }

std::string ConjuntoDeReglas::texto_del_consecuente(int i) const {
  return variables_.nombre_de_etiqueta(variables_.consecuente(), clase(i));
}

std::string ConjuntoDeReglas::texto(int i) const {
  const ReglaCodificada& regla = regla_i(i);
  const genetico::Codigo& codigo = regla.codigo;
  std::ostringstream salida;
  salida << "SI\n";
  for (const AtributoRelevante& relevante : relevantes(codigo)) {
    salida << '\t' << variables_.nombre(relevante.atributo) << " = {";
    for (std::size_t t = 0; t < relevante.n_casos; ++t) {
      if (codigo.etiquetas[relevante.donde + t] == '1') {
        salida << ' ' << variables_.nombre_de_etiqueta(relevante.atributo, static_cast<int>(t));
      }
    }
    salida << "}\n";
  }

  const int consecuente = variables_.consecuente();
  salida << "ENTONCES " << variables_.nombre(consecuente) << " ES "
         << variables_.nombre_de_etiqueta(consecuente, clase(i)) << "   P " << regla.peso
         << " y N+= " << regla.aciertos << " y N-= " << regla.fallos << '\n';
  return salida.str();
}

void ConjuntoDeReglas::anadir_a_fichero(const std::string& nomfich) const {
  std::ofstream fichero(nomfich, std::ios::app);
  comun::progreso(Verbosidad::depuracion) << "Numero de reglas =" << numero_de_reglas() << '\n';
  for (int i = 0; i < numero_de_reglas(); ++i) {
    fichero << texto(i) << '\n';
  }
}

// La parte general de SplitAntecedent, que estaba en old/: la interpretación de PlanMiner (solo
// atributos de hasta dos etiquetas, con el nombre de la etiqueta o su centro) se hace fuera.
std::vector<Condicion> ConjuntoDeReglas::antecedente(int i) const {
  const genetico::Codigo& codigo = regla_i(i).codigo;
  std::vector<Condicion> condiciones;
  for (const AtributoRelevante& relevante : relevantes(codigo)) {
    Condicion condicion;
    condicion.atributo = indice(relevante.atributo);
    for (std::size_t t = 0; t < relevante.n_casos; ++t) {
      if (codigo.etiquetas[relevante.donde + t] == '1') {
        condicion.etiquetas.push_back(t);
      }
    }
    condiciones.push_back(condicion);
  }
  return condiciones;
}

}  // namespace inslv::reglas
