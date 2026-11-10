# Licencias

[English](LICENSES.md) · **Español**

El código de este repositorio y la documentación se distribuyen con la
[licencia pública general de GNU, versión 3](LICENSE), solo esa versión (`GPL-3.0-only`).

## NSLV

INSLV parte de NSLV, de Raúl Pérez (© 2002–2003), con la licencia GNU GPL versión 2 o posterior,
como dicen las cabeceras de sus fuentes. Esa opción de usar una versión posterior es la que
permite distribuirlo con la versión 3.

## Datos de prueba

- `tests/datos/nslv/`: irs, wne y aut son Iris, Wine y Automobile, del repositorio de aprendizaje
  automático de la UCI. ring es Ringnorm, el problema artificial de Leo Breiman, en la versión de
  KEEL (que lo tomó de DELVE). Siguen las condiciones de sus originales. boxA es de Raúl Pérez, que
  lo hizo mientras desarrollaba NSLV para ver si un algoritmo detecta relaciones complejas entre
  atributos: no es un conjunto público de referencia.
- `tests/datos/keel/`: las particiones de Automobile en el formato de KEEL. Siguen las
  condiciones de su original.
- `tests/datos/planminer/`: los casos salen de las trazas de prueba de blocksworld y depots del
  PlanMiner original. Siguen las condiciones de sus originales.
