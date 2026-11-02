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

std::string nombre_del_tipo(TipoDeReglas tipo) {
  return tipo == TipoDeReglas::descriptivas ? "descriptivas" : "discriminantes";
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

Datos Experimento::entrenamiento() const {
  const fs::path directorio = datos / "nslv" / conjunto;
  std::vector<fs::path> particiones;
  for (int i = 0; i < 10; ++i) {
    if (i != test) {
      particiones.push_back(directorio / (conjunto + std::to_string(i) + ".datos"));
    }
  }
  return nslv::leer(directorio / (conjunto + ".dom"), particiones);
}

Datos Experimento::prueba() const {
  const fs::path directorio = datos / "nslv" / conjunto;
  return nslv::leer(directorio / (conjunto + ".dom"),
                    {directorio / (conjunto + std::to_string(test) + ".datos")});
}

Opciones Experimento::opciones() const {
  Opciones resultado;
  resultado.reglas = tipo;
  resultado.tamano_episodio = episodio;
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
  experimento.tipo =
      partes[3] == "descriptivas" ? TipoDeReglas::descriptivas : TipoDeReglas::discriminantes;
  if (partes[5].matched) {
    experimento.episodio = std::stoul(partes[5]);
  }
  return experimento;
}

std::string describir(const std::vector<Regla>& reglas) {
  std::ostringstream salida;
  salida << "reglas " << reglas.size() << '\n';
  for (const Regla& regla : reglas) {
    salida << "regla " << regla.consecuente << ' ' << exacto(regla.cubiertos) << ' '
           << exacto(regla.peso) << ' ' << regla.antecedente.size() << '\n';
    for (const Condicion& condicion : regla.antecedente) {
      salida << "condicion " << condicion.atributo;
      for (const std::size_t etiqueta : condicion.etiquetas) {
        salida << ' ' << etiqueta;
      }
      salida << '\n';
    }
  }
  return salida.str();
}

std::string describir(const std::vector<std::optional<std::size_t>>& clases) {
  std::ostringstream salida;
  salida << "predicciones " << clases.size() << '\n';
  for (std::size_t e = 0; e < clases.size(); ++e) {
    salida << (e == 0 ? "" : " ");
    const std::optional<std::size_t>& clase = clases[e];
    if (clase.has_value()) {
      salida << *clase;
    } else {
      salida << -1;
    }
  }
  salida << '\n';
  return salida.str();
}

void escribir_reglas(const fs::path& fichero, const std::vector<Regla>& reglas,
                     const std::vector<std::optional<std::size_t>>& clases) {
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
         << describir(reglas) << "# Las clases que da old/ a los ejemplos de la partición "
         << experimento.test << " con estas reglas (-1: ninguna regla dispara).\n"
         << describir(clases);
}

std::vector<Regla> leer_reglas(const fs::path& fichero) {
  std::ifstream entrada(fichero);
  if (!entrada) {
    throw std::runtime_error("no se puede abrir " + fichero.string());
  }
  std::size_t n_reglas = 0;
  siguiente_linea(entrada, "reglas", fichero) >> n_reglas;
  std::vector<Regla> reglas(n_reglas);
  for (Regla& regla : reglas) {
    std::istringstream linea = siguiente_linea(entrada, "regla", fichero);
    std::string cubiertos;
    std::string peso;
    std::size_t condiciones = 0;
    linea >> regla.consecuente >> cubiertos >> peso >> condiciones;
    regla.cubiertos = leer_numero(cubiertos);
    regla.peso = leer_numero(peso);
    regla.antecedente.resize(condiciones);
    for (Condicion& condicion : regla.antecedente) {
      std::istringstream linea_condicion = siguiente_linea(entrada, "condicion", fichero);
      linea_condicion >> condicion.atributo;
      std::size_t etiqueta = 0;
      while (linea_condicion >> etiqueta) {
        condicion.etiquetas.push_back(etiqueta);
      }
    }
  }
  return reglas;
}

std::vector<std::optional<std::size_t>> leer_predicciones(const fs::path& fichero) {
  std::ifstream entrada(fichero);
  std::string linea;
  while (std::getline(entrada, linea) && !linea.starts_with("predicciones ")) {
  }
  if (!entrada) {
    throw std::runtime_error(fichero.string() + ": no tiene predicciones");
  }
  std::istringstream cabecera(linea);
  std::string palabra;
  std::size_t n = 0;
  cabecera >> palabra >> n;
  std::vector<std::optional<std::size_t>> clases;
  long clase = 0;
  while (clases.size() < n && entrada >> clase) {
    clases.push_back(clase < 0 ? std::nullopt
                               : std::optional<std::size_t>(static_cast<std::size_t>(clase)));
  }
  if (clases.size() != n) {
    throw std::runtime_error(fichero.string() + ": faltan predicciones");
  }
  return clases;
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
