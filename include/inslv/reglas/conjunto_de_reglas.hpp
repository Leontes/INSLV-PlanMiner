// SPDX-License-Identifier: GPL-3.0-only
// Un conjunto de reglas, con su inferencia. Parte de NSLV, de Raúl Pérez (2002).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "inslv/datos/ejemplo.hpp"
#include "inslv/difuso/variables.hpp"
#include "inslv/genetico/codigo.hpp"
#include "inslv/inslv.hpp"

namespace inslv::reglas {

/// Las reglas, en orden, codificadas como individuos del genético (genetico::Codigo).
///
/// Cada regla tiene un peso, que sale de los ejemplos que acierta (N+) y los que falla (N-):
/// (N+ + 1) / (N+ + N- + 1).
///
/// Todo lo que recibe una regla lanza std::logic_error si no existe, salvo Remove(i).
class ConjuntoDeReglas {
 public:
  /// Las variables de los atributos y la clase.
  void poner_variables(const difuso::Variables& x);

  /// Añade una regla al final, con ese peso (N+ y N- a 0) y como modificada.
  void anadir(const genetico::Codigo& x, double peso);
  /// Añade una regla al final, con el peso que sale de sus N+ y N-.
  void anadir(const genetico::Codigo& x, double mas, double menos, bool modif);
  /// Añade al principio una copia de las reglas de r.
  void anadir_al_principio(const ConjuntoDeReglas& r);
  /// Quita la última regla.
  void quitar_ultima();
  /// Quita la regla i, si existe.
  void quitar(int i);

  /// La regla i (se puede cambiar). La referencia deja de valer al añadir o quitar reglas.
  [[nodiscard]] genetico::Codigo& codigo(int i);
  [[nodiscard]] const genetico::Codigo& codigo(int i) const;

  /// Suma a N+ y N- de la regla y recalcula su peso.
  void sumar_ejemplos(int i, double mas, double menos);
  [[nodiscard]] double peso(int i) const;
  /// Recalcula el peso de una regla, o de todas, con sus N+ y N-.
  void recalcular_peso(int i);
  void recalcular_peso();
  /// Suman a N+ y a N- de la regla, sin recalcular su peso.
  void sumar_aciertos(int i, double value);
  void sumar_fallos(int i, double value);
  [[nodiscard]] double aciertos(int i) const;
  [[nodiscard]] double fallos(int i) const;
  /// Si la regla ha cambiado y hay que volver a calcular su peso.
  void marcar_modificada(int i, bool value);
  [[nodiscard]] bool modificada(int i) const;
  void marcar_todas_modificadas();

  [[nodiscard]] int numero_de_reglas() const;
  /// Cuántas reglas tienen un peso de al menos minimo.
  [[nodiscard]] int numero_con_peso(double minimo) const;
  /// La media de atributos con nivel por encima del umbral por regla.
  [[nodiscard]] double atributos_por_regla() const;
  /// Los atributos con nivel por encima del umbral de la regla.
  [[nodiscard]] int atributos_sobre_el_umbral(int i) const;
  /// Las etiquetas que no admiten los atributos con nivel por encima del umbral de la regla.
  [[nodiscard]] int etiquetas_excluidas(int i) const;
  /// En frec[atributo], en cuántas reglas es relevante (ver antecedente). Devuelve la proporción
  /// de atributos que lo son en alguna.
  double frecuencia_de_atributos(std::vector<int>& frec) const;
  /// El total de condiciones de las reglas (ver antecedente).
  [[nodiscard]] int numero_de_condiciones() const;

  /// La regla con más adaptación al ejemplo por su peso (a igualdad, la de más peso y, si no, la
  /// primera), en regla_disparada, y su clase. Si no se adapta ninguna, -1 en las dos; en grado,
  /// esa adaptación, y si no se adapta ninguna, no lo cambia.
  int inferir(const datos::Ejemplo& v, double& grado, int& regla_disparada) const;
  /// Igual, para un ejemplo de esa clase. Además, en grado_p y peso_p, la adaptación y el peso de
  /// la mejor regla de su clase; en grado_n y peso_n, los de la mejor de otra. Y en aislada, si lo
  /// clasifica bien, cuántas reglas de su clase ganan a las de otra; si no, cuántas de otra ganan
  /// a las de su clase, en negativo (-1000 si no se le adapta ninguna de su clase).
  int inferir(const datos::Ejemplo& v, int clase_del_ejemplo, double& grado_p, double& peso_p,
              double& grado_n, double& peso_n, int& regla_disparada, int& aislada) const;

  [[nodiscard]] int clase(int i) const;
  /// El texto de la regla, para el informe, o el de su consecuente.
  [[nodiscard]] std::string texto(int i) const;
  [[nodiscard]] std::string texto_del_consecuente(int i) const;
  /// Añade el texto de las reglas al final del fichero.
  void anadir_a_fichero(const std::string& nomfich) const;

  /// Las condiciones de la regla: los atributos relevantes (con nivel por encima del umbral y sin
  /// admitir todas sus etiquetas) con las etiquetas que admiten.
  [[nodiscard]] std::vector<Condicion> antecedente(int i) const;

 private:
  struct ReglaCodificada {
    genetico::Codigo codigo;
    double peso = 0;
    double aciertos = 0;
    double fallos = 0;
    bool modificada = true;
  };

  // Un atributo relevante de una regla: dónde empiezan sus etiquetas en Binary(0) y cuántas son.
  struct AtributoRelevante {
    int atributo;
    std::size_t donde;
    std::size_t n_casos;
  };

  void comprobar(int i) const;
  [[nodiscard]] const ReglaCodificada& regla_i(int i) const;
  [[nodiscard]] ReglaCodificada& regla_i(int i);
  // Lo que se adapta el ejemplo a la regla, por su peso.
  [[nodiscard]] double adaptacion_por_peso(const datos::Ejemplo& v,
                                           const ReglaCodificada& regla) const;
  [[nodiscard]] std::vector<AtributoRelevante> relevantes(const genetico::Codigo& codigo) const;

  std::vector<ReglaCodificada> reglas_;
  difuso::Variables variables_;
};

}  // namespace inslv::reglas
