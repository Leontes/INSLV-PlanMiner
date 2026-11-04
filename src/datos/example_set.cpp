// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/datos/example_set.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace inslv {

namespace {

std::size_t indice(int i) { return static_cast<std::size_t>(i); }

}  // namespace

example_set::example_set(int ejemplos, int variables) {
  // En el original solo avisaba y dejaba los punteros sin inicializar.
  if (ejemplos < 0 || variables <= 0) {
    throw std::logic_error("inslv: example_set con " + std::to_string(ejemplos) + " ejemplos y " +
                           std::to_string(variables) + " variables");
  }
  n_example = ejemplos;
  n_variable = variables;
  const std::size_t n = indice(ejemplos);
  data.assign(n, std::vector<double>(indice(variables)));
  covered.assign(n, false);
  g_positive_covered.assign(n, 0);
  g_negative_covered.assign(n, 0);
  peso_positive.assign(n, 0);
  peso_negative.assign(n, 0);
  rule_used.assign(n, -1);
  rule_aislada.assign(n, 0);
}

example_set::example_set(const std::vector<std::vector<double>>& columnas)
    : example_set(static_cast<int>(columnas[0].size()), static_cast<int>(columnas.size())) {
  for (std::size_t v = 0; v < columnas.size(); ++v) {
    for (std::size_t e = 0; e < columnas[v].size(); ++e) {
      data[e][v] = columnas[v][e];
    }
  }
}

double example_set::Data(int ejemplo, int variable) const {
  return data[indice(ejemplo)][indice(variable)];
}

vectordouble example_set::Data(int ejemplo) const { return vectordouble(data[indice(ejemplo)]); }

int example_set::N_Examples() const { return n_example; }

int example_set::Not_Covered_Examples() const {
  int numero = 0;
  for (const bool cubierto : covered) {
    if (!cubierto) {
      ++numero;
    }
  }
  return numero;
}

void example_set::Examples_per_Class(int VarClass, int nclasses, int* n_examples_in_class) const {
  for (int i = 0; i < nclasses; ++i) {
    n_examples_in_class[i] = 0;
  }
  for (std::size_t i = 0; i < data.size(); ++i) {
    if (!covered[i]) {
      ++n_examples_in_class[static_cast<int>(data[i][indice(VarClass)])];
    }
  }
}

void example_set::Mark(const int* v, int tama, const double* grado_p, const double* peso_p,
                       const double* grado_n, const double* peso_n, const int* rule,
                       const int* r_aislada) {
  for (int i = 0; i < tama; ++i) {
    const std::size_t e = indice(i);
    covered[e] = v[i] == 1;
    g_positive_covered[e] = grado_p[i];
    peso_positive[e] = peso_p[i];
    g_negative_covered[e] = grado_n[i];
    peso_negative[e] = peso_n[i];
    rule_used[e] = rule[i];
    rule_aislada[e] = r_aislada[i];
  }
}

void example_set::UnMarkAll() {
  covered.assign(covered.size(), false);
  g_positive_covered.assign(g_positive_covered.size(), 0);
  g_negative_covered.assign(g_negative_covered.size(), 0);
  peso_positive.assign(peso_positive.size(), 0);
  peso_negative.assign(peso_negative.size(), 0);
  rule_used.assign(rule_used.size(), -1);
  rule_aislada.assign(rule_aislada.size(), 0);
}

bool example_set::Is_Covered(int i) const { return covered[indice(i)]; }

double example_set::Grade_Is_Positive_Covered(int i) const { return g_positive_covered[indice(i)]; }

double example_set::Grade_Is_Negative_Covered(int i) const { return g_negative_covered[indice(i)]; }

double example_set::Peso_Is_Positive_Covered(int i) const { return peso_positive[indice(i)]; }

double example_set::Peso_Is_Negative_Covered(int i) const { return peso_negative[indice(i)]; }

int example_set::Class_of_example(int VarClase, int i) const {
  return static_cast<int>(data[indice(i)][indice(VarClase)]);
}

int example_set::Rule_Used(int i) const { return rule_used[indice(i)]; }

int example_set::Rule_Aislada(int i) const { return rule_aislada[indice(i)]; }

template <typename Condicion>
example_set example_set::Seleccionar(Condicion condicion) const {
  std::vector<std::size_t> elegidos;
  for (std::size_t e = 0; e < data.size(); ++e) {
    if (condicion(e)) {
      elegidos.push_back(e);
    }
  }
  example_set resultado(static_cast<int>(elegidos.size()), n_variable);
  for (std::size_t i = 0; i < elegidos.size(); ++i) {
    resultado.data[i] = data[elegidos[i]];
  }
  return resultado;
}

example_set example_set::SubSet(int tama) const {
  return Seleccionar([&](std::size_t e) { return e < indice(tama); });
}

example_set example_set::SubSet(int ini, int fin) const {
  return Seleccionar([&](std::size_t e) { return indice(ini) <= e && e <= indice(fin); });
}

}  // namespace inslv
