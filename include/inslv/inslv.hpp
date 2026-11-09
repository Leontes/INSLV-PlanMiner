// SPDX-License-Identifier: GPL-3.0-only
// INSLV: aprendizaje de reglas difusas con el algoritmo genético de NSLV, en su versión
// incremental (los ejemplos se procesan por episodios), y clasificación con esas reglas.
//
// Esta es la cabecera de la interfaz. Las de las sublibrerías (inslv/<sublibrería>/) son de la
// implementación y pueden cambiar. inslv/formatos/nslv.hpp lee conjuntos de datos en el formato
// de NSLV.
#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "inslv/generador_de_azar.hpp"

namespace inslv {

/// Etiqueta difusa trapezoidal [a, b, c, d], con a <= b <= c <= d. La adaptación de un valor es 1
/// en [b, c], sube de 0 a 1 entre a y b, baja de 1 a 0 entre c y d, y es 0 fuera. El centro de la
/// etiqueta es (b + c) / 2. Las etiquetas no se extienden hasta el infinito por los extremos.
struct ConjuntoDifuso {
  std::string etiqueta;
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  double d = 0.0;
};

/// Una variable con su partición difusa.
struct Variable {
  std::string nombre;
  std::vector<ConjuntoDifuso> etiquetas;
};

/// Un conjunto de ejemplos, por columnas.
///
/// - valores[i][e]: valor del atributo i en el ejemplo e. NaN es desconocido: se adapta por
///   completo a todas las etiquetas.
/// - clases[e]: la clase del ejemplo e, como índice de una etiqueta de la clase.
struct Datos {
  std::vector<Variable> atributos;
  Variable clase;
  std::vector<std::vector<double>> valores;
  std::vector<std::size_t> clases;
};

/// Una condición del antecedente: el atributo toma alguna de las etiquetas (índices en
/// Datos::atributos y Variable::etiquetas).
struct Condicion {
  std::size_t atributo = 0;
  std::vector<std::size_t> etiquetas;
};

/// Una regla: si se cumplen todas las condiciones, el ejemplo es de la clase consecuente.
struct Regla {
  std::vector<Condicion> antecedente;
  std::size_t consecuente = 0;
  /// Ejemplos positivos que cubre la regla (n+ en NSLV).
  double cubiertos = 0.0;
  /// Peso de la regla al clasificar: gana la regla con mayor adaptación del ejemplo por su peso.
  double peso = 0.0;
};

/// Qué versión de cada regla devuelve aprender. El genético obtiene las dos primeras a la vez, con
/// la misma bondad.
enum class TipoDeReglas {
  /// La más específica con esa bondad. Son las que usa PlanMiner.
  descriptivas,
  /// La más general: solo las condiciones mínimas del antecedente.
  discriminantes,
  /// La descriptiva con todas las condiciones que cumplen sus ejemplos, aunque cuesten bondad,
  /// salvo
  /// una fracción tolerancia_al_ruido de ellos. Con ruido, a las descriptivas les faltan
  /// condiciones
  /// que se cumplen en casi todos sus ejemplos. Experimental (fase B).
  caracteristicas,
};

/// Cuánto cuenta aprender de lo que hace. Cada nivel incluye los anteriores.
enum class Verbosidad {
  /// Nada.
  ninguna,
  /// Una línea al empezar (ejemplos, atributos, clases y episodios) y otra al acabar (reglas y
  /// tiempo).
  resumen,
  /// Además, una línea por episodio: sus ejemplos, las reglas añadidas, el acierto antes y después
  /// y el tiempo.
  episodios,
  /// Además, lo de dentro de cada episodio: las medidas de información, el umbral, el genético y
  /// el análisis de patrones.
  detalle,
  /// Todo, incluidos los volcados de las variables y de la población inicial.
  depuracion,
};

struct Opciones {
  /// Semilla del generador de números aleatorios. Con la misma semilla y los mismos datos, las
  /// mismas reglas.
  unsigned semilla = 0;
  /// El generador que usar; si es nulo, uno propio. Se siembra con semilla al empezar: pasarlo solo
  /// hace falta para seguir su secuencia después de aprender.
  GeneradorDeAzar* azar = nullptr;
  /// Qué versión de las reglas se devuelve.
  TipoDeReglas reglas = TipoDeReglas::descriptivas;
  /// Con TipoDeReglas::caracteristicas, la fracción de los ejemplos de una regla que puede
  /// incumplir una condición que se le añade (desde 0, sin llegar a 1).
  double tolerancia_al_ruido = 0;
  /// Ejemplos por episodio del aprendizaje incremental (entre 1 y el máximo de int).
  std::size_t tamano_episodio = 5000;
  /// Cuánto se cuenta del aprendizaje.
  Verbosidad verbosidad = Verbosidad::ninguna;
  /// Adónde va lo que pide verbosidad; si es nulo, a la salida estándar.
  std::ostream* progreso = nullptr;
  /// Si no está vacío, el informe del aprendizaje (medidas y reglas) se escribe en este fichero.
  std::filesystem::path informe;
};

/// Aprende las reglas de los ejemplos.
///
/// Lanza std::invalid_argument si los datos no son coherentes (tamaños distintos, variables sin
/// etiquetas, etiquetas que no cumplen a <= b <= c <= d, valores que no se adaptan a ninguna
/// etiqueta de su atributo o clases fuera de rango) o el tamaño del episodio no es válido.
///
/// No tiene estado global: se puede llamar a la vez desde varios hilos, siempre que cada uno use
/// sus datos, su generador y su flujo de progreso.
std::vector<Regla> aprender(const Datos& datos, const Opciones& opciones = {});

/// Clasifica los ejemplos con las reglas, como la inferencia de NSLV: gana la regla con mayor
/// adaptación del ejemplo por su peso; si empatan, la de más peso y, si no, la primera. La
/// adaptación a una regla es la menor de sus condiciones. La adaptación a una condición es la de
/// su etiqueta a la que se adapta el valor o, si se adapta a dos o más, 1 (supone que las etiquetas
/// forman una partición). Un valor desconocido se adapta por completo. Nada si ninguna regla tiene
/// adaptación positiva.
///
/// Los atributos de datos tienen que ser los del aprendizaje; sus clases no se usan (pueden estar
/// vacías). Lanza std::invalid_argument si los valores no son coherentes con los atributos o una
/// regla usa un atributo o una etiqueta que no existen.
std::vector<std::optional<std::size_t>> clasificar(const std::vector<Regla>& reglas,
                                                   const Datos& datos);

}  // namespace inslv
