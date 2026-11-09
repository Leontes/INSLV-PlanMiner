# INSLV

[English](README.md) · **Español**

Aprendizaje de reglas difusas con el algoritmo genético de NSLV, en su versión incremental: los
ejemplos se procesan por episodios. INSLV aprende las reglas que
[PlanMiner](https://github.com/Leontes/PlanMiner) convierte en las precondiciones y los efectos
de las acciones de planificación.

Este repositorio porta INSLV a C++ moderno (C++20). **En desarrollo:** la implementación sigue
siendo la original, sin su código muerto y con sus fallos corregidos, detrás de una interfaz nueva
en castellano (`inslv::aprender` e `inslv::clasificar`). Se moderniza paso a paso, y cada paso
tiene que dar exactamente las mismas reglas que el original.

## Qué añade al original

- **Las mismas reglas**, comprobadas contra el original en cada paso, y las mismas clases que su
  inferencia.
- **Reglas descriptivas y discriminantes.** INSLV estaba ajustado para dar reglas descriptivas,
  las que necesita PlanMiner; ahora da también las discriminantes (solo las condiciones mínimas del
  antecedente).
- **Clasificación** (`clasificar`), no solo aprendizaje de reglas.
- **Un lector del formato de NSLV** (dominios `.dom` y ejemplos `.datos`).
- **Sin fugas de memoria, sin llamadas que terminen el programa y sin tocar la salida estándar**
  salvo que se pida, con una verbosidad por niveles.
- **Sin estado global:** el generador aleatorio es propio y se puede llamar a `aprender` desde varios
  hilos.

## Uso

```cpp
#include "inslv/inslv.hpp"

inslv::Datos datos;
datos.atributos = {{"a", {{"False", 0, 0, 0, 0}, {"True", 1, 1, 1, 1}}}};
datos.clase = {"Clase", {{"Negativa", 0, 0, 0, 0}, {"Positiva", 1, 1, 1, 1}}};
datos.valores = {{0, 1, 1, 0}};  // valores[atributo][ejemplo]; NaN: desconocido
datos.clases = {0, 1, 1, 0};     // índice de una etiqueta de la clase

std::vector<inslv::Regla> reglas = inslv::aprender(datos);
for (const inslv::Regla& regla : reglas) {
  // regla.antecedente: atributos y etiquetas que admiten; regla.consecuente: la clase;
  // regla.cubiertos: ejemplos positivos cubiertos; regla.peso: su peso al clasificar
}
// Una clase por ejemplo, o ninguna si no dispara ninguna regla.
std::vector<std::optional<std::size_t>> clases = inslv::clasificar(reglas, datos);
```

Las opciones (`inslv::Opciones`) eligen las reglas descriptivas (las de por defecto, las que usa
PlanMiner) o las discriminantes, con solo las condiciones mínimas; el tamaño del episodio; y cuánto
cuenta el aprendizaje (`verbosidad`, de `ninguna` a `depuracion`) y dónde (`progreso`, un
`std::ostream*`; por defecto, la salida estándar). Los conjuntos de datos en el formato de NSLV se
leen con `inslv/formatos/nslv.hpp`:

```cpp
#include "inslv/formatos/nslv.hpp"

inslv::Datos datos = inslv::nslv::leer("irs.dom", {"irs1.datos", "irs2.datos"});
inslv::Opciones opciones;
opciones.reglas = inslv::TipoDeReglas::discriminantes;
opciones.tamano_episodio = 50;  // ejemplos por episodio (5000 por defecto)
opciones.verbosidad = inslv::Verbosidad::episodios;  // una línea por episodio
std::vector<inslv::Regla> reglas = inslv::aprender(datos, opciones);
```

```cmake
add_subdirectory(inslv)  # o FetchContent
target_link_libraries(mi_programa PRIVATE inslv::inslv)
```

Ver [Cómo aprende INSLV](docs/es/algoritmo.md).

## Estructura

| Ruta | Qué hay |
|------|---------|
| `include/inslv/` | la cabecera pública (`inslv.hpp`) y, en una carpeta por sublibrería, las cabeceras internas |
| `src/` | la interfaz (`inslv.cpp`) y las sublibrerías, cada una en su carpeta: |
| `src/datos/` | los ejemplos |
| `src/difuso/` | la lógica difusa: etiquetas, dominios y variables |
| `src/genetico/` | el algoritmo genético: el código genético, las poblaciones y sus operadores |
| `src/reglas/` | el conjunto de reglas y su inferencia |
| `src/aprendizaje/` | el aprendizaje incremental de NSLV |
| `src/comun/` | lo que comparten las sublibrerías: el flujo del progreso |
| `src/formatos/` | el lector del formato de NSLV, aparte del motor |
| `tests/` | tests unitarios, de equivalencia y del oráculo; `apoyo/`: lo que comparten |
| `tests/datos/nslv/` | cinco conjuntos de datos en el formato de NSLV, cada uno con su dominio y 10 particiones de validación cruzada: aut, boxA, irs, ring y wne |
| `tests/datos/keel/` | las particiones de aut en el formato de KEEL, para probar el lector |
| `tests/datos/planminer/` | 26 casos de equivalencia: la entrada de INSLV en el PlanMiner original y las reglas que devolvió |
| `tests/datos/referencias/` | las reglas que da el original con los conjuntos de NSLV |
| `docs/es/`, `docs/en/` | la documentación, en castellano y en inglés |
| `cmake/` | módulos de CMake: avisos, sanitizers, cobertura, sublibrerías, dependencias y el oráculo |

`build/` (compilaciones), `old/` (el código original, el oráculo de los tests) y `privado/`
(documentos internos) no están en git.

## Documentación

- [Cómo aprende INSLV](docs/es/algoritmo.md): qué aprende, cómo clasifica, los episodios,
  el genético y los niveles de verbosidad.
- [Tests](docs/es/pruebas.md): etiquetas, datos de prueba, reglas de referencia, el oráculo,
  sanitizers y cobertura.
- [Licencias](LICENCIAS.md): la del código, la de NSLV y las de los datos de prueba.

## Desarrollo

```bash
cmake --workflow --preset debug                     # configurar, compilar y probar
cmake --workflow --preset asan                      # con AddressSanitizer y UBSan
cmake --workflow --preset cobertura                 # cobertura (GCC) y después el informe:
cmake --build --preset cobertura --target informe_cobertura  # build/cobertura/cobertura/
cmake --preset tidy && cmake --build --preset tidy  # clang-tidy sobre el código nuevo
git ls-files -- '*.cpp' '*.hpp' '*.h' | xargs clang-format --dry-run --Werror
```

- **Tests:** por etiquetas de CTest (`ctest --preset debug -L unidad`): `unidad` (la interfaz y el
  lector), `equivalencia` (las reglas frente a las que dio el original, guardadas en
  `tests/datos`), `lento` (lo mismo con los conjuntos grandes: `ctest --preset lento`) y
  `oraculo` (solo en local, con `old/`: la librería frente al original compilado en el mismo
  programa). Ver [Tests](docs/es/pruebas.md).
- **Avisos:** todo se compila con avisos estrictos, y la integración continua los trata como
  errores.
- **Dependencias de los tests:** GoogleTest 1.14 o posterior (si no está instalado, CMake lo
  descarga); para la cobertura, gcovr.
- **Integración continua:** compila y prueba con GCC y Clang, pasa los tests lentos, los
  sanitizers y clang-tidy, y comprueba el formato (clang-format 18).

## Licencia

El código y la documentación se distribuyen con la
[licencia pública general de GNU, versión 3](LICENSE) (`GPL-3.0-only`). INSLV parte de NSLV,
de Raúl Pérez (© 2002–2003); el port es © 2026 José Segura. Los conjuntos de datos de prueba
públicos siguen las condiciones de sus originales. Ver [LICENCIAS.md](LICENCIAS.md).
