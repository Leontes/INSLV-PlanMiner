# Tests

[English](../en/tests.md) · **Español**

Los tests van por etiquetas de CTest:

```bash
ctest --preset debug                    # todos menos los lentos
ctest --preset debug -L equivalencia    # una etiqueta
ctest --preset debug -R 'unidad.Nslv.*' # por nombre
ctest --preset lento                    # los lentos, en Release
```

| Etiqueta | Qué comprueba | Necesita |
|----------|---------------|----------|
| `unidad` | la interfaz (`aprender`, `clasificar`, las opciones y la verbosidad) y el lector del formato de NSLV | GoogleTest (si no está instalado, CMake lo descarga) |
| `equivalencia` | que `aprender` da las mismas reglas que el original y `clasificar`, las mismas clases: los casos de PlanMiner y las reglas de referencia rápidas | — |
| `lento` | las reglas de referencia de aut, boxA y ring, que tardan minutos | — |
| `oraculo` | la librería frente al original compilado en el mismo programa | `old/`, solo en local |

Ningún preset de tests pasa los lentos salvo el suyo (`lento`), ni los presets de sanitizers y de
cobertura pasan el oráculo.

## Los datos de prueba

En `tests/datos`:

| Ruta | Qué hay |
|------|---------|
| `nslv/` | cinco conjuntos en el formato de NSLV, cada uno con su dominio (`.dom`) y 10 particiones de validación cruzada (`<nombre>0.datos` a `<nombre>9.datos`) |
| `keel/aut/` | las particiones de aut en el formato de KEEL: otra partición de los mismos ejemplos, para comprobar el lector |
| `planminer/` | 26 casos de equivalencia: la entrada de INSLV en el PlanMiner original (blocksworld y depots) y las reglas que devolvió, ya interpretadas |
| `referencias/` | las reglas que da el original con los conjuntos de `nslv/`: `rapidas/` y `lentas/` |

| Conjunto | Ejemplos | Atributos | Clases | Notas |
|----------|---------:|----------:|-------:|-------|
| irs | 150 | 4 | 3 | |
| wne | 178 | 13 | 3 | |
| aut | 205 | 25 | 6 | valores desconocidos, atributos nominales de hasta 22 etiquetas |
| boxA | 1000 | 6 | 3 | de Raúl Pérez, para probar NSLV |
| ring | 7400 | 20 | 2 | Ringnorm de Breiman, versión de KEEL; con 9 particiones pasa de un episodio (5000 ejemplos) |

## Las reglas de referencia

Cada fichero de `tests/datos/referencias` tiene las reglas que dio el original en un experimento,
que dice su nombre, y las clases que da su inferencia con ellas a los ejemplos de la partición de
test:

```
<conjunto>-test<k>-<descriptivas|discriminantes>[-episodio<n>].reglas
```

Se aprende con todas las particiones del conjunto salvo la `k`, con la semilla por defecto y
episodios de `n` ejemplos (5000 si no se dice). El test aprende lo mismo y compara las reglas bit a
bit (la clase, los ejemplos cubiertos, el peso y cada condición con sus etiquetas) y lo que da
`clasificar` con ellas a la partición `k`.

| Grupo | Experimentos |
|-------|--------------|
| `rapidas/` | irs con cada una de las 10 particiones como test; wne con la 0; irs y wne con episodios de 20 y 50 ejemplos (la parte incremental). Descriptivas y discriminantes |
| `lentas/` | aut, boxA y ring con la partición 0, descriptivas y discriminantes |

Se generan con el original, así que solo se pueden regenerar en local, con `old/`:

```bash
cmake --build --preset debug --target inslv_generar_referencias
ls tests/datos/referencias/*/*.reglas \
  | xargs -P "$(nproc)" -n 1 build/debug/tests/oraculo/inslv_generar_referencias
```

Para añadir un experimento basta con pasarle el nombre del fichero nuevo. El original usa el
generador aleatorio global y desvía la salida estándar: cada fichero se genera en su propio proceso.

## El oráculo

`old/` es la implementación original, que solo está en local. Si está, CMake la compila como
`inslv_legacy`, con el espacio de nombres cambiado por una macro (`-Dinslv=inslv_legacy`) para
poder enlazarla en el mismo programa que la librería, y registra los tests de la etiqueta
`oraculo`: con cada caso de PlanMiner, las reglas descriptivas y las discriminantes tienen que ser
bit a bit las del original, y `clasificar` tiene que dar a los ejemplos las mismas clases que su
inferencia (`ruleset::InferenceC`). Se desactiva con `-DINSLV_ORACULO=OFF`.

## Sanitizers y cobertura

```bash
cmake --workflow --preset asan       # AddressSanitizer y UBSan
cmake --workflow --preset cobertura  # cobertura con gcov (GCC)
cmake --build --preset cobertura --target informe_cobertura
```

- **Sanitizers:** sin supresiones: cualquier fuga de memoria o comportamiento indefinido hace
  fallar los tests. Las tres fugas del original (unos 800 bytes por llamada) están corregidas.
- **Cobertura:** se instrumentan la librería y sus sublibrerías, no los tests. El informe queda en
  `build/cobertura/cobertura/` (`index.html` y `resumen.txt`); necesita gcovr.

## Integración continua

Compila y pasa los tests con GCC y con Clang (avisos como errores), los tests lentos, los
sanitizers, clang-tidy y la comprobación del formato (clang-format 18). No tiene `old/`: la
equivalencia con el original se comprueba con los casos y las reglas de referencia guardados.
