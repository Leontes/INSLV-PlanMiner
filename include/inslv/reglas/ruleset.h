// SPDX-License-Identifier: GPL-3.0-only
// Un conjunto de reglas, con su inferencia. Parte de NSLV, de Raúl Pérez (2002).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "inslv/datos/ejemplo.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/genetcode.h"
#include "inslv/inslv.hpp"

namespace inslv {

/// Las reglas, en orden, codificadas como individuos del genético (genetcode).
///
/// Cada regla tiene un peso, que sale de los ejemplos que acierta (N+) y los que falla (N-):
/// (N+ + 1) / (N+ + N- + 1).
///
/// Todo lo que recibe una regla lanza std::logic_error si no existe, salvo Remove(i).
class ruleset {
 public:
  /// Las variables de los atributos y la clase.
  void AddDomain(const difuso::Variables& x);

  /// Añade una regla al final, con ese peso (N+ y N- a 0) y como modificada.
  void Add(const genetcode& x, double weight);
  /// Añade una regla al final, con el peso que sale de sus N+ y N-.
  void Add(const genetcode& x, double mas, double menos, bool modif);
  /// Añade al principio una copia de las reglas de r.
  void AddRuleset(const ruleset& r);
  /// Quita la última regla.
  void Remove();
  /// Quita la regla i, si existe.
  void Remove(int i);

  /// La regla i (se puede cambiar). La referencia deja de valer al añadir o quitar reglas.
  [[nodiscard]] genetcode& Get_Code_Rule(int i);
  [[nodiscard]] const genetcode& Get_Code_Rule(int i) const;

  /// Suma a N+ y N- de la regla y recalcula su peso.
  void Add_Weight(int i, double mas, double menos);
  [[nodiscard]] double Get_Weight(int i) const;
  /// Recalcula el peso de una regla, o de todas, con sus N+ y N-.
  void Recalculate_Weight(int i);
  void Recalculate_Weight();
  /// Suman a N+ y a N- de la regla, sin recalcular su peso.
  void Add_N_Mas(int i, double value);
  void Add_N_Menos(int i, double value);
  [[nodiscard]] double Get_N_Mas(int i) const;
  [[nodiscard]] double Get_N_Menos(int i) const;
  /// Si la regla ha cambiado y hay que volver a calcular su peso.
  void Modificada(int i, bool value);
  [[nodiscard]] bool Get_Modificada(int i) const;
  void FixModifyToTrue();

  [[nodiscard]] int N_rule() const;
  /// Cuántas reglas tienen un peso de al menos weight.
  [[nodiscard]] int N_rule(double weight) const;
  /// La media de atributos con nivel por encima del umbral por regla.
  [[nodiscard]] double Variables_per_rule() const;
  /// Los atributos con nivel por encima del umbral de la regla.
  [[nodiscard]] int Variables_of_rule(int i) const;
  /// Las etiquetas que no admiten los atributos con nivel por encima del umbral de la regla.
  [[nodiscard]] int Numero_Ceros_of_rule(int i) const;
  /// En frec[atributo], en cuántas reglas es relevante (ver Antecedente). Devuelve la proporción
  /// de atributos que lo son en alguna.
  double Frecuence_each_Variables(std::vector<int>& frec) const;
  /// El total de condiciones de las reglas (ver Antecedente).
  [[nodiscard]] int Condiciones_per_RB() const;

  /// La regla con más adaptación al ejemplo por su peso (a igualdad, la de más peso y, si no, la
  /// primera), en regla_disparada, y su clase. Si no se adapta ninguna, -1 en las dos; en grado,
  /// esa adaptación, y si no se adapta ninguna, no lo cambia.
  int InferenceC(const datos::Ejemplo& v, double& grado, int& regla_disparada) const;
  /// Igual, para un ejemplo de esa clase. Además, en grado_p y peso_p, la adaptación y el peso de
  /// la mejor regla de su clase; en grado_n y peso_n, los de la mejor de otra. Y en aislada, si lo
  /// clasifica bien, cuántas reglas de su clase ganan a las de otra; si no, cuántas de otra ganan
  /// a las de su clase, en negativo (-1000 si no se le adapta ninguna de su clase).
  int InferenceC(const datos::Ejemplo& v, int clase, double& grado_p, double& peso_p,
                 double& grado_n, double& peso_n, int& regla_disparada, int& aislada) const;

  [[nodiscard]] int Class_of_rule(int i) const;
  /// El texto de la regla, para el informe, o el de su consecuente.
  [[nodiscard]] std::string SPrint(int i) const;
  [[nodiscard]] std::string SPrint_Consequent(int i) const;
  /// Añade el texto de las reglas al final del fichero.
  void SaveRuleInterpreted_append(const std::string& nomfich) const;

  /// Las condiciones de la regla: los atributos relevantes (con nivel por encima del umbral y sin
  /// admitir todas sus etiquetas) con las etiquetas que admiten.
  [[nodiscard]] std::vector<Condicion> Antecedente(int i) const;

 private:
  struct ReglaCodificada {
    genetcode codigo;
    double peso = 0;
    double n_mas = 0;
    double n_menos = 0;
    bool modificada = true;
  };

  // Un atributo relevante de una regla: dónde empiezan sus etiquetas en Binary(0) y cuántas son.
  struct AtributoRelevante {
    int atributo;
    std::size_t donde;
    std::size_t n_casos;
  };

  void Comprobar(int i) const;
  [[nodiscard]] const ReglaCodificada& Regla_i(int i) const;
  [[nodiscard]] ReglaCodificada& Regla_i(int i);
  // Lo que se adapta el ejemplo a la regla, por su peso.
  [[nodiscard]] double Adaptacion(const datos::Ejemplo& v, const ReglaCodificada& regla) const;
  [[nodiscard]] std::vector<AtributoRelevante> Relevantes(const genetcode& code) const;

  std::vector<ReglaCodificada> reglas_;
  difuso::Variables variables_;
};

}  // namespace inslv
