// SPDX-License-Identifier: GPL-3.0-only
/***************************************************************************
                          ruleset.cpp  -  description
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

#include "inslv/reglas/ruleset.h"

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

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

double Peso(double mas, double menos) { return (mas + 1) / (mas + menos + 1); }

// Los atributos con nivel por encima del umbral.
int SobreElUmbral(const genetico::Codigo& code) {
  const std::vector<double>& niveles = code.niveles;
  return static_cast<int>(std::count_if(niveles.begin(), niveles.end() - 1,
                                        [&](double nivel) { return nivel >= niveles.back(); }));
}

}  // namespace

void ruleset::Comprobar(int i) const {
  // En el original no se comprobaba: leía otra memoria o un puntero sin inicializar.
  if (i < 0 || i >= N_rule()) {
    throw std::logic_error("inslv: no existe la regla " + std::to_string(i));
  }
}

const ruleset::ReglaCodificada& ruleset::Regla_i(int i) const {
  Comprobar(i);
  return reglas_[indice(i)];
}

ruleset::ReglaCodificada& ruleset::Regla_i(int i) {
  Comprobar(i);
  return reglas_[indice(i)];
}

void ruleset::AddDomain(const difuso::Variables& x) { variables_ = x; }

void ruleset::Add(const genetico::Codigo& x, double weight) {
  reglas_.push_back({x, weight, 0, 0, true});
}

void ruleset::Add(const genetico::Codigo& x, double mas, double menos, bool modif) {
  reglas_.push_back({x, Peso(mas, menos), mas, menos, modif});
}

void ruleset::AddRuleset(const ruleset& r) {
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

void ruleset::Remove() { reglas_.pop_back(); }

void ruleset::Remove(int i) {
  if (i >= 0 && i < N_rule()) {
    reglas_.erase(reglas_.begin() + i);
  }
}

genetico::Codigo& ruleset::Get_Code_Rule(int i) { return Regla_i(i).codigo; }

const genetico::Codigo& ruleset::Get_Code_Rule(int i) const { return Regla_i(i).codigo; }

void ruleset::Add_Weight(int i, double mas, double menos) {
  ReglaCodificada& regla = Regla_i(i);
  regla.n_mas += mas;
  regla.n_menos += menos;
  regla.peso = Peso(regla.n_mas, regla.n_menos);
}

double ruleset::Get_Weight(int i) const { return Regla_i(i).peso; }

void ruleset::Recalculate_Weight(int i) {
  ReglaCodificada& regla = Regla_i(i);
  regla.peso = Peso(regla.n_mas, regla.n_menos);
}

void ruleset::Recalculate_Weight() {
  for (ReglaCodificada& regla : reglas_) {
    regla.peso = Peso(regla.n_mas, regla.n_menos);
  }
}

void ruleset::Add_N_Mas(int i, double value) { Regla_i(i).n_mas += value; }

void ruleset::Add_N_Menos(int i, double value) { Regla_i(i).n_menos += value; }

double ruleset::Get_N_Mas(int i) const { return Regla_i(i).n_mas; }

double ruleset::Get_N_Menos(int i) const { return Regla_i(i).n_menos; }

void ruleset::Modificada(int i, bool value) { Regla_i(i).modificada = value; }

bool ruleset::Get_Modificada(int i) const { return Regla_i(i).modificada; }

void ruleset::FixModifyToTrue() {
  for (ReglaCodificada& regla : reglas_) {
    regla.modificada = true;
  }
}

int ruleset::N_rule() const { return static_cast<int>(reglas_.size()); }

int ruleset::N_rule(double weight) const {
  return static_cast<int>(std::count_if(reglas_.begin(), reglas_.end(),
                                        [&](const auto& regla) { return regla.peso >= weight; }));
}

double ruleset::Variables_per_rule() const {
  int total = 0;
  for (const ReglaCodificada& regla : reglas_) {
    total += SobreElUmbral(regla.codigo);
  }
  return 1.0 * total / N_rule();
}

int ruleset::Variables_of_rule(int i) const { return SobreElUmbral(Regla_i(i).codigo); }

int ruleset::Numero_Ceros_of_rule(int i) const {
  const genetico::Codigo& code = Regla_i(i).codigo;
  const std::size_t atributos = code.niveles.size() - 1;
  int ceros = 0;
  std::size_t donde = 0;
  for (std::size_t j = 0; j < atributos; ++j) {
    const auto n_casos = indice(variables_.numero_de_etiquetas(static_cast<int>(j)));
    if (code.niveles[j] >= code.niveles.back()) {
      const std::string_view etiquetas = std::string_view(code.etiquetas).substr(donde, n_casos);
      ceros += static_cast<int>(std::count(etiquetas.begin(), etiquetas.end(), '0'));
    }
    donde += n_casos;
  }
  return ceros;
}

std::vector<ruleset::AtributoRelevante> ruleset::Relevantes(const genetico::Codigo& code) const {
  std::vector<AtributoRelevante> relevantes;
  std::size_t donde = 0;
  for (int j = 0; j < variables_.numero_de_atributos(); ++j) {
    const auto n_casos = indice(variables_.numero_de_etiquetas(j));
    const std::string_view etiquetas = std::string_view(code.etiquetas).substr(donde, n_casos);
    if (code.niveles[indice(j)] >= code.niveles.back() &&
        !std::all_of(etiquetas.begin(), etiquetas.end(), [](char bit) { return bit == '1'; })) {
      relevantes.push_back({j, donde, n_casos});
    }
    donde += n_casos;
  }
  return relevantes;
}

double ruleset::Frecuence_each_Variables(std::vector<int>& frec) const {
  // En el original leía la regla 0 para saber cuántos atributos hay, aunque no hubiera reglas.
  frec.assign(indice(variables_.numero_de_atributos()), 0);
  for (const ReglaCodificada& regla : reglas_) {
    for (const AtributoRelevante& relevante : Relevantes(regla.codigo)) {
      ++frec[indice(relevante.atributo)];
    }
  }
  const auto usados = std::count_if(frec.begin(), frec.end(), [](int veces) { return veces > 0; });
  return static_cast<double>(usados) / static_cast<double>(frec.size());
}

int ruleset::Condiciones_per_RB() const {
  std::size_t condiciones = 0;
  for (const ReglaCodificada& regla : reglas_) {
    condiciones += Relevantes(regla.codigo).size();
  }
  return static_cast<int>(condiciones);
}

double ruleset::Adaptacion(const datos::Ejemplo& v, const ReglaCodificada& regla) const {
  return variables_.adaptacion(v, regla.codigo.etiquetas, regla.codigo.niveles) * regla.peso;
}

int ruleset::InferenceC(const datos::Ejemplo& v, double& grado, int& regla_disparada) const {
  double max = 0;
  int re = -1;
  for (int i = 0; i < N_rule(); ++i) {
    const ReglaCodificada& regla = Regla_i(i);
    const double aux = Adaptacion(v, regla);
    if (aux > max || (re != -1 && aux > 0 && aux == max && regla.peso > Regla_i(re).peso)) {
      max = aux;
      re = i;
      grado = aux;
    }
  }
  regla_disparada = re;
  return re != -1 ? Class_of_rule(re) : -1;
}

int ruleset::InferenceC(const datos::Ejemplo& v, int clase, double& grado_p, double& peso_p,
                        double& grado_n, double& peso_n, int& regla_disparada, int& aislada) const {
  const int n_rule = N_rule();
  // Lo que se adapta a cada regla: en positivo, las de su clase; en negativo, las de otra.
  std::vector<double> adaptacion_particular;
  adaptacion_particular.reserve(reglas_.size());
  double max = 0;
  int re = -1;
  bool clasifica_bien = false;
  int indice_pos = n_rule;
  int indice_neg = n_rule;
  grado_p = 0;
  peso_p = 0;
  grado_n = 0;
  peso_n = 0;

  for (int i = 0; i < n_rule; ++i) {
    const ReglaCodificada& regla = Regla_i(i);
    const double aux = Adaptacion(v, regla);
    const bool de_su_clase = Class_of_rule(i) == clase;
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
    if (aux > max || (re != -1 && aux > 0 && aux == max && regla.peso > Regla_i(re).peso)) {
      max = aux;
      re = i;
      clasifica_bien = de_su_clase;
    }
  }

  // Si la regla i, con esa adaptación, gana a la mejor de la otra parte.
  const auto gana = [&](int i, double adaptacion, double grado, double peso, int mejor) {
    const double peso_i = Regla_i(i).peso;
    return adaptacion > grado || (adaptacion == grado && peso_i > peso) ||
           (adaptacion == grado && peso_i == peso && i < mejor);
  };

  if (clasifica_bien) {
    aislada = 0;
    for (int i = 0; i < n_rule; ++i) {
      const double aux = adaptacion_particular[indice(i)];
      if (aux > 0 && gana(i, aux, grado_n, peso_n, indice_neg)) {
        ++aislada;
      }
    }
    if (aislada == 0) {
      std::ostream& salida = comun::progreso(Verbosidad::depuracion);
      salida << "Error!!!!  Clase " << clase << "  Grado Pos: " << grado_p
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
    for (int i = 0; i < n_rule; ++i) {
      const double aux = adaptacion_particular[indice(i)];
      if (aux < 0 && gana(i, -aux, grado_p, peso_p, indice_pos)) {
        --aislada;
      }
    }
  }

  regla_disparada = re;
  return re != -1 ? Class_of_rule(re) : -1;
}

int ruleset::Class_of_rule(int i) const { return Regla_i(i).codigo.clase; }

std::string ruleset::SPrint_Consequent(int i) const {
  return variables_.nombre_de_etiqueta(variables_.consecuente(), Class_of_rule(i));
}

std::string ruleset::SPrint(int i) const {
  const ReglaCodificada& regla = Regla_i(i);
  const genetico::Codigo& code = regla.codigo;
  std::ostringstream buffer;
  buffer << "IF\n";
  for (const AtributoRelevante& relevante : Relevantes(code)) {
    buffer << '\t' << variables_.nombre(relevante.atributo) << " = {";
    for (std::size_t t = 0; t < relevante.n_casos; ++t) {
      if (code.etiquetas[relevante.donde + t] == '1') {
        buffer << ' ' << variables_.nombre_de_etiqueta(relevante.atributo, static_cast<int>(t));
      }
    }
    buffer << "}\n";
  }

  const int consecuente = variables_.consecuente();
  buffer << "THEN " << variables_.nombre(consecuente) << " IS "
         << variables_.nombre_de_etiqueta(consecuente, Class_of_rule(i)) << "   W " << regla.peso
         << " and N+= " << regla.n_mas << " and N-= " << regla.n_menos << '\n';
  return buffer.str();
}

void ruleset::SaveRuleInterpreted_append(const std::string& nomfich) const {
  std::ofstream fichero(nomfich, std::ios::app);
  comun::progreso(Verbosidad::depuracion) << "Numero de reglas =" << N_rule() << '\n';
  for (int i = 0; i < N_rule(); ++i) {
    fichero << SPrint(i) << '\n';
  }
}

// La parte general de SplitAntecedent, que estaba en old/: la interpretación de PlanMiner (solo
// atributos de hasta dos etiquetas, con el nombre de la etiqueta o su centro) se hace fuera.
std::vector<Condicion> ruleset::Antecedente(int i) const {
  const genetico::Codigo& code = Regla_i(i).codigo;
  std::vector<Condicion> condiciones;
  for (const AtributoRelevante& relevante : Relevantes(code)) {
    Condicion condicion;
    condicion.atributo = indice(relevante.atributo);
    for (std::size_t t = 0; t < relevante.n_casos; ++t) {
      if (code.etiquetas[relevante.donde + t] == '1') {
        condicion.etiquetas.push_back(t);
      }
    }
    condiciones.push_back(condicion);
  }
  return condiciones;
}

}  // namespace inslv
