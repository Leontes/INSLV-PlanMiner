// SPDX-License-Identifier: GPL-3.0-only
#include "referencias.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include "inslv/formatos/nslv.hpp"

namespace inslv::pruebas {

namespace {

namespace fs = std::filesystem;

const fs::path datos = INSLV_DATOS;

// La representación más corta de x que se lee igual.
std::string exacto(double x) {
  std::array<char, 32> texto{};
  const auto [fin, codigo] = std::to_chars(texto.data(), texto.data() + texto.size(), x);
  if (codigo != std::errc()) {
    throw std::runtime_error("no se puede escribir un número");
  }
  return {texto.data(), fin};
}

double leer_numero(const std::string& texto) {
  double valor = 0;
  const char* const fin = texto.data() + texto.size();
  const auto [ultimo, codigo] = std::from_chars(texto.data(), fin, valor);
  if (codigo != std::errc() || ultimo != fin) {
    throw std::runtime_error("no es un número: " + texto);
  }
  return valor;
}

std::string nombre_del_tipo(RuleType tipo) {
  return tipo == RuleType::descriptive ? "descriptivas" : "discriminantes";
}

// La siguiente línea que no es un comentario, que tiene que empezar por la palabra clave.
std::istringstream siguiente_linea(std::ifstream& entrada, const std::string& clave,
                                   const fs::path& fichero) {
  std::string texto;
  do {
    if (!std::getline(entrada, texto)) {
      throw std::runtime_error(fichero.string() + ": se acaba el fichero; se esperaba '" + clave +
                               "'");
    }
  } while (texto.empty() || texto.front() == '#');
  std::istringstream linea(texto);
  std::string palabra;
  linea >> palabra;
  if (palabra != clave) {
    throw std::runtime_error(fichero.string() + ": se esperaba '" + clave + "' y hay: " + texto);
  }
  return linea;
}

}  // namespace

Dataset Experimento::entrenamiento() const {
  const fs::path directorio = datos / "nslv" / conjunto;
  std::vector<fs::path> particiones;
  for (int i = 0; i < 10; ++i) {
    if (i != test) {
      particiones.push_back(directorio / (conjunto + std::to_string(i) + ".datos"));
    }
  }
  return nslv::leer(directorio / (conjunto + ".dom"), particiones);
}

Options Experimento::opciones() const {
  Options resultado;
  resultado.rules = tipo;
  resultado.episode_size = episodio;
  return resultado;
}

Experimento experimento_de(const fs::path& fichero) {
  static const std::regex formato(
      R"(([A-Za-z0-9]+)-test([0-9])-(descriptivas|discriminantes)(-episodio([0-9]+))?\.reglas)");
  const std::string nombre = fichero.filename().string();
  std::smatch partes;
  if (!std::regex_match(nombre, partes, formato)) {
    throw std::runtime_error(nombre + " no es <conjunto>-test<k>-<tipo>[-episodio<n>].reglas");
  }
  Experimento experimento;
  experimento.conjunto = partes[1];
  experimento.test = std::stoi(partes[2]);
  experimento.tipo = partes[3] == "descriptivas" ? RuleType::descriptive : RuleType::discriminant;
  if (partes[5].matched) {
    experimento.episodio = std::stoul(partes[5]);
  }
  return experimento;
}

std::string describir(const std::vector<Rule>& reglas) {
  std::ostringstream salida;
  salida << "reglas " << reglas.size() << '\n';
  for (const Rule& regla : reglas) {
    salida << "regla " << regla.consequent << ' ' << exacto(regla.covered) << ' '
           << exacto(regla.weight) << ' ' << regla.antecedent.size() << '\n';
    for (const Condition& condicion : regla.antecedent) {
      salida << "condicion " << condicion.attribute;
      for (const std::size_t etiqueta : condicion.labels) {
        salida << ' ' << etiqueta;
      }
      salida << '\n';
    }
  }
  return salida.str();
}

void escribir_reglas(const fs::path& fichero, const std::vector<Rule>& reglas) {
  const Experimento experimento = experimento_de(fichero);
  std::ofstream salida(fichero);
  if (!salida) {
    throw std::runtime_error("no se puede escribir " + fichero.string());
  }
  salida << "# Reglas de referencia de INSLV: las que da old/ al aprender " << experimento.conjunto
         << " sin la partición " << experimento.test << ",\n# reglas "
         << nombre_del_tipo(experimento.tipo) << ", episodios de " << experimento.episodio
         << " ejemplos y la semilla por defecto.\n"
         << "# Generado por inslv_generar_referencias (tests/oraculo): no editar a mano.\n"
         << describir(reglas);
}

std::vector<Rule> leer_reglas(const fs::path& fichero) {
  std::ifstream entrada(fichero);
  if (!entrada) {
    throw std::runtime_error("no se puede abrir " + fichero.string());
  }
  std::size_t n_reglas = 0;
  siguiente_linea(entrada, "reglas", fichero) >> n_reglas;
  std::vector<Rule> reglas(n_reglas);
  for (Rule& regla : reglas) {
    std::istringstream linea = siguiente_linea(entrada, "regla", fichero);
    std::string cubiertos;
    std::string peso;
    std::size_t condiciones = 0;
    linea >> regla.consequent >> cubiertos >> peso >> condiciones;
    regla.covered = leer_numero(cubiertos);
    regla.weight = leer_numero(peso);
    regla.antecedent.resize(condiciones);
    for (Condition& condicion : regla.antecedent) {
      std::istringstream linea_condicion = siguiente_linea(entrada, "condicion", fichero);
      linea_condicion >> condicion.attribute;
      std::size_t etiqueta = 0;
      while (linea_condicion >> etiqueta) {
        condicion.labels.push_back(etiqueta);
      }
    }
  }
  return reglas;
}

std::vector<fs::path> ficheros_de_referencia(const std::string& grupo) {
  std::vector<fs::path> ficheros;
  for (const auto& entrada : fs::directory_iterator(datos / "referencias" / grupo)) {
    if (entrada.path().extension() == ".reglas") {
      ficheros.push_back(entrada.path());
    }
  }
  std::sort(ficheros.begin(), ficheros.end());
  return ficheros;
}

}  // namespace inslv::pruebas
