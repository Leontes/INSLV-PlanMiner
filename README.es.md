# INSLV

[English](README.md) · **Español**

Aprendizaje de reglas difusas con el algoritmo genético de NSLV, en su versión incremental: los
ejemplos se procesan por episodios. INSLV aprende las reglas que
[PlanMiner](https://github.com/Leontes/PlanMiner) convierte en las precondiciones y los efectos
de las acciones de planificación.

Este repositorio porta INSLV a C++ moderno (C++20). **En desarrollo:** aquí todavía no hay
código. Hasta que el port esté listo, la versión en uso es la de PlanMiner (`vendor/inslv`).

## Objetivos

- **Las mismas reglas que el original**, comprobadas contra él en cada paso.
- **Reglas descriptivas y discriminantes.** INSLV estaba ajustado para dar reglas descriptivas,
  las que necesita PlanMiner; la versión nueva también dará reglas discriminantes (solo las
  condiciones mínimas del antecedente).
- **Clasificación**, no solo aprendizaje de reglas.
- **Un lector del formato de NSLV** (dominios `.dom` y ejemplos `.datos`).
- Una librería sin estado global, sin fugas de memoria y sin llamadas que terminen el programa.

## Estructura

| Ruta | Qué hay |
|------|---------|
| `tests/datos/nslv/` | cinco conjuntos de datos en el formato de NSLV, cada uno con su dominio y 10 particiones de validación cruzada: aut, boxA, irs, ring y wne |
| `tests/datos/planminer/` | 26 casos de equivalencia: la entrada de INSLV en el PlanMiner original y las reglas que devolvió |

`build/` (compilaciones), `old/` (el código original, el oráculo de los tests) y `privado/`
(documentos internos) no están en git.

## Licencia

El código y la documentación se distribuyen con la
[licencia pública general de GNU, versión 3](LICENSE). INSLV parte de NSLV, de Raúl Pérez
(© 2002–2003); el port es © 2026 José Segura. Los conjuntos de datos de prueba siguen las
condiciones de sus originales. Ver [LICENCIAS.md](LICENCIAS.md).
