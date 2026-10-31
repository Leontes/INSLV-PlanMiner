// SPDX-License-Identifier: GPL-3.0-only
#include "inslv/formatos/nslv.hpp"

#include <charconv>
#include <cstddef>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>

namespace inslv::nslv {

namespace {

// El valor que marca un dato desconocido en los ficheros de ejemplos.
constexpr double desconocido = -999999999;

// Lee un fichero palabra a palabra (separadas por espacios en blanco), sabiendo en qué línea
// está cada una para los mensajes de error.
class Lector {
 public:
  explicit Lector(std::filesystem::path fichero)
      : fichero_(std::move(fichero)), entrada_(fichero_) {
    if (!entrada_) {
      throw ErrorDeFormato(fichero_.string() + ": no se puede abrir");
    }
  }

  // La siguiente palabra. que: lo que se espera, para el mensaje si se acaba el fichero.
  std::string palabra(const std::string& que) {
    std::string resultado;
    while (!(linea_actual_ >> resultado)) {
      std::string texto;
      if (!std::getline(entrada_, texto)) {
        error("se acaba el fichero y falta " + que);
      }
      ++linea_;
      linea_actual_ = std::istringstream(texto);
    }
    return resultado;
  }

  double real(const std::string& que) {
    const std::string texto = palabra(que);
    // from_chars no admite el signo +.
    const std::size_t inicio = texto.starts_with('+') ? 1 : 0;
    double valor = 0;
    const char* const fin = texto.data() + texto.size();
    const auto [ultimo, codigo] = std::from_chars(texto.data() + inicio, fin, valor);
    if (codigo != std::errc() || ultimo != fin) {
      error(que + " no es un número: " + texto);
    }
    return valor;
  }

  std::size_t natural(const std::string& que) {
    const std::string texto = palabra(que);
    std::size_t valor = 0;
    const char* const fin = texto.data() + texto.size();
    const auto [ultimo, codigo] = std::from_chars(texto.data(), fin, valor);
    if (codigo != std::errc() || ultimo != fin) {
      error(que + " no es un número natural: " + texto);
    }
    return valor;
  }

  // Comprueba que no queda nada más en el fichero.
  void final() {
    std::string sobra;
    while (!(linea_actual_ >> sobra)) {
      std::string texto;
      if (!std::getline(entrada_, texto)) {
        return;
      }
      ++linea_;
      linea_actual_ = std::istringstream(texto);
    }
    error("sobra texto al final: " + sobra);
  }

  [[noreturn]] void error(const std::string& mensaje) const {
    throw ErrorDeFormato(fichero_.string() + ":" + std::to_string(linea_) + ": " + mensaje);
  }

 private:
  std::filesystem::path fichero_;
  std::ifstream entrada_;
  std::istringstream linea_actual_;
  std::size_t linea_ = 0;
};

// Un dominio leído: las variables, sin ejemplos, y la columna de la clase en los ficheros de
// ejemplos.
struct Dominio {
  Dataset datos;
  std::size_t columna_clase = 0;
  std::size_t variables = 0;
};

// a <= b <= c <= d. Escrito así, también es falso si alguno es NaN.
bool es_trapecio(const FuzzySet& etiqueta) {
  return etiqueta.a <= etiqueta.b && etiqueta.b <= etiqueta.c && etiqueta.c <= etiqueta.d;
}

Variable leer_variable(Lector& lector, const std::string& nombre) {
  Variable variable;
  variable.name = nombre;
  const std::string de = " de " + nombre;
  lector.real("la unidad" + de);
  lector.real("el factor" + de);
  lector.real("el mínimo" + de);
  lector.real("el máximo" + de);
  const std::size_t etiquetas = lector.natural("el número de etiquetas" + de);
  if (etiquetas == 0) {
    lector.error(nombre + " no tiene etiquetas");
  }
  for (std::size_t i = 0; i < etiquetas; ++i) {
    FuzzySet etiqueta;
    const std::string de_etiqueta = " de la etiqueta " + std::to_string(i) + de;
    etiqueta.a = lector.real("a" + de_etiqueta);
    etiqueta.b = lector.real("b" + de_etiqueta);
    etiqueta.c = lector.real("c" + de_etiqueta);
    etiqueta.d = lector.real("d" + de_etiqueta);
    etiqueta.label = lector.palabra("el nombre" + de_etiqueta);
    if (!es_trapecio(etiqueta)) {
      lector.error("la etiqueta " + etiqueta.label + de + " no cumple a <= b <= c <= d");
    }
    variable.labels.push_back(std::move(etiqueta));
  }
  return variable;
}

Dominio leer_dominio_con_clase(const std::filesystem::path& fichero) {
  Lector lector(fichero);
  Dominio dominio;
  dominio.variables = lector.natural("el número de variables");
  std::optional<std::size_t> clase;
  for (std::size_t i = 0; i < dominio.variables; ++i) {
    const std::string nombre = lector.palabra("el nombre de la variable " + std::to_string(i));
    const std::size_t es_clase = lector.natural("si " + nombre + " es la clase");
    if (es_clase > 1) {
      lector.error("si " + nombre + " es la clase tiene que ser 0 o 1");
    }
    if (es_clase == 1 && clase) {
      lector.error("hay dos clases: " + dominio.datos.target.name + " y " + nombre);
    }
    Variable variable = leer_variable(lector, nombre);
    if (es_clase == 0) {
      dominio.datos.attributes.push_back(std::move(variable));
    } else {
      clase = i;
      dominio.datos.target = std::move(variable);
    }
  }
  lector.final();
  if (!clase) {
    lector.error("ninguna variable es la clase");
  }
  dominio.columna_clase = *clase;
  dominio.datos.values.resize(dominio.datos.attributes.size());
  return dominio;
}

// La etiqueta de la clase cuyo centro es el valor.
std::optional<std::size_t> etiqueta_de_clase(const Variable& clase, double valor) {
  for (std::size_t i = 0; i < clase.labels.size(); ++i) {
    const FuzzySet& etiqueta = clase.labels[i];
    if ((etiqueta.b + etiqueta.c) / 2.0 == valor) {
      return i;
    }
  }
  return std::nullopt;
}

void leer_ejemplos(const std::filesystem::path& fichero, const Dominio& dominio, Dataset& datos) {
  Lector lector(fichero);
  const std::size_t ejemplos = lector.natural("el número de ejemplos");
  const std::size_t variables = lector.natural("el número de variables");
  if (variables != dominio.variables) {
    lector.error("hay " + std::to_string(variables) + " variables y el dominio tiene " +
                 std::to_string(dominio.variables));
  }
  for (std::size_t e = 0; e < ejemplos; ++e) {
    std::size_t atributo = 0;
    for (std::size_t v = 0; v < variables; ++v) {
      const double valor =
          lector.real("el valor " + std::to_string(v) + " del ejemplo " + std::to_string(e));
      if (v == dominio.columna_clase) {
        const std::optional<std::size_t> clase = etiqueta_de_clase(datos.target, valor);
        if (!clase) {
          lector.error("la clase del ejemplo " + std::to_string(e) + " (" + std::to_string(valor) +
                       ") no es el centro de ninguna etiqueta de " + datos.target.name);
        }
        datos.classes.push_back(*clase);
      } else {
        datos.values[atributo++].push_back(
            valor == desconocido ? std::numeric_limits<double>::quiet_NaN() : valor);
      }
    }
  }
  lector.final();
}

}  // namespace

Dataset leer_dominio(const std::filesystem::path& dominio) {
  return leer_dominio_con_clase(dominio).datos;
}

Dataset leer(const std::filesystem::path& dominio,
             const std::vector<std::filesystem::path>& ejemplos) {
  const Dominio leido = leer_dominio_con_clase(dominio);
  Dataset datos = leido.datos;
  for (const std::filesystem::path& fichero : ejemplos) {
    leer_ejemplos(fichero, leido, datos);
  }
  return datos;
}

}  // namespace inslv::nslv
