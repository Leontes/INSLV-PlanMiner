# Cómo aprende INSLV

[English](../en/algorithm.md) · **Español**

INSLV aprende reglas difusas con el algoritmo genético de NSLV, en una versión incremental que
procesa los ejemplos por episodios. Esta página cuenta qué aprende, cómo clasifica y cómo funciona
el aprendizaje, a grandes rasgos y con los nombres del código.

## Qué aprende

Una regla dice: si cada atributo de su antecedente toma alguna de sus etiquetas, el ejemplo es de
su clase.

```
si x1 es {Baja, Media} y x3 es {Alta} entonces Iris-virginica   (peso 0.97)
```

- **Etiquetas difusas.** Cada atributo tiene su partición en etiquetas trapezoidales `[a, b, c, d]`.
  La adaptación de un valor a una etiqueta es 1 en `[b, c]`, sube de 0 a 1 entre `a` y `b`, baja
  de 1 a 0 entre `c` y `d`, y es 0 fuera. Un valor desconocido se adapta por completo a todas.
- **Condiciones con varias etiquetas.** La adaptación a una condición es la de su etiqueta a la que
  se adapta el valor o, si se adapta a dos o más, 1: supone que las etiquetas forman una partición,
  así que la unión de dos que se solapan cubre el valor por completo. La adaptación a una regla es
  la menor de sus condiciones. Los atributos que no aparecen no cuentan.
- **Peso.** Cada regla tiene un peso que sale de los ejemplos que cubre: `(n⁺ + 1) / (n⁺ + n⁻ + 1)`,
  con `n⁺` y `n⁻` la suma de las adaptaciones de los ejemplos de su clase y de los de las demás.
- **Descriptivas y discriminantes.** El genético obtiene cada regla en dos versiones con la misma
  bondad: la más general (discriminante: solo las condiciones mínimas para separar la clase) y la
  más específica (descriptiva: con todas las condiciones que puede añadir sin perder bondad).
  `Opciones::reglas` elige cuáles se devuelven; las descriptivas son las que usa PlanMiner.
- **Características.** Con ruido, a las descriptivas les faltan condiciones que se
  cumplen en casi todos sus ejemplos: como cada una deja fuera alguno, cuesta bondad y no entra. Las
  características (`TipoDeReglas::caracteristicas`) son las descriptivas con todas las condiciones que
  cumple al menos un `1 - Opciones::tolerancia_al_ruido` de los ejemplos de su clase que cubren. Se
  calculan al final, sobre todos los ejemplos: el aprendizaje es el mismo.
- **Comparaciones entre atributos.** Con `Opciones::comparaciones`, una condición puede comparar
  dos atributos de la misma unidad (`Variable::unidad`, distinta de 0; el lector de NSLV la toma
  del dominio): `x1 < x2`, `x1 = x2`, `x1 <= x2`... Antes de aprender, cada par de atributos de la
  misma unidad da un atributo nuevo, el resultado de compararlos (menor, igual o mayor), que se
  queda si su información mutua con alguna clase pasa de `Opciones::umbral_de_comparaciones` (0,2)
  por la mayor de los atributos con esa clase. El genético los trata como a los demás, y en las
  reglas quedan como condiciones con `Condicion::comparado_con`. Sirve cuando la clase depende de
  una relación entre atributos, que con etiquetas de un solo atributo solo se puede aproximar con
  muchas reglas pequeñas: en boxA (si una caja cabe en la otra), el acierto en test pasa del 68 % al
  99 % y las reglas, de 77 a 15. En los demás conjuntos de prueba, el acierto no cambia más allá de
  lo que varía con la semilla.

## Cómo clasifica

`clasificar` da a cada ejemplo la clase de la regla con mayor adaptación por peso. Si empatan,
gana la de más peso y, si también empatan, la primera. Si ninguna regla tiene adaptación
positiva, el ejemplo se queda sin clase. Es la inferencia de NSLV, y da las mismas clases que el
original. Una comparación se cumple (1) o no (0); con un valor desconocido, se cumple.

## Cómo aprende

### Los episodios

Los ejemplos se procesan en bloques de `Opciones::tamano_episodio` (5000 por defecto), en el
orden en que llegan. En cada episodio:

1. Se mide el acierto de las reglas que ya hay sobre los ejemplos del episodio y se ajustan sus
   pesos.
2. Si el acierto no llega a un umbral (al principio, el 100 %), se quitan los ejemplos que las
   reglas ya clasifican bien y el genético aprende reglas nuevas con los que quedan.
3. Se analizan las reglas nuevas junto a las anteriores: se combinan las de la misma clase cuyos
   patrones encajan y se señalan las contradicciones.
4. Si las reglas nuevas no mejoran el acierto sobre el episodio, se descartan.

Con un solo episodio (menos ejemplos que su tamaño), el genético aprende con todos los ejemplos a
la vez.

### El genético

El genético busca una regla cada vez (cobertura secuencial): la mejor que encuentra se añade y se
vuelve a empezar mientras siga añadiendo reglas.

- **Codificación.** Cada individuo combina una parte binaria (las etiquetas que admite cada
  atributo), una entera (la clase) y una real (la relevancia de cada atributo y un umbral: un
  atributo con relevancia menor que el umbral no entra en la regla).
- **Población inicial.** Las relevancias empiezan en las medidas de información de cada atributo
  respecto de cada clase, calculadas con los ejemplos (al azar si son todas iguales); el umbral, en
  0,2.
- **Bondad.** Un vector de criterios que se comparan en orden. El principal mide cuánto mejora la
  clasificación de los ejemplos del episodio al añadir la regla: los ejemplos de su clase que pasa
  a clasificar bien menos los de otras clases que pasa a clasificar mal, ponderados por su
  adaptación y por el peso de la regla.
- **Las dos versiones.** De cada regla se obtiene la versión más general y la más específica con
  la misma bondad; se guardan emparejadas.

### El generador aleatorio

Usa un generador propio (`GeneradorDeAzar`), sembrado con `Opciones::semilla`: con la misma semilla y
los mismos datos, las mismas reglas, en cualquier plataforma. Da la misma secuencia que `srand` y
`rand` de glibc, porque las reglas dependen de ella, pero no tiene estado global: no toca el
generador del programa y se puede llamar a `aprender` a la vez desde varios hilos.

Quien quiera seguir su secuencia después de aprender puede pasar su generador en `Opciones::azar`.
Es lo que hace falta para reproducir lo que hacía un programa que seguía usando `rand()` después de
llamar al original.

## Lo que cuenta mientras aprende

`Opciones::verbosidad` elige cuánto se escribe en `Opciones::progreso` (o en la salida estándar):

| Nivel | Qué escribe |
|-------|-------------|
| `ninguna` | nada (por defecto) |
| `resumen` | una línea al empezar (ejemplos, atributos, clases y episodios) y otra al acabar (reglas y tiempo) |
| `episodios` | además, una línea por episodio: sus ejemplos, las reglas antes y después, el acierto antes y después y el tiempo |
| `detalle` | además, lo de dentro de cada episodio: las medidas de información, el umbral, los ejemplos eliminados, las reglas añadidas y el análisis de patrones |
| `depuracion` | todo, incluidos los volcados de las variables, de la población inicial y de los patrones |

```
INSLV: 135 ejemplos, 4 atributos y 3 clases; 2 episodios de hasta 70 ejemplos; reglas descriptivas
INSLV: episodio 1 de 2 (ejemplos 0 a 69): reglas 0 -> 4, acierto 0.000 -> 0.986, 0.79 s
INSLV: episodio 2 de 2 (ejemplos 70 a 134): reglas 4 -> 6, acierto 0.938 -> 1.000, 0.40 s
INSLV: 6 reglas en 1.20 s
```

La verbosidad no cambia las reglas.

## Respecto al original

Con los mismos datos, la misma semilla y las opciones por defecto, `aprender` da bit a bit las
mismas reglas que el INSLV original, y `clasificar`, las mismas clases que su inferencia (ver
[Tests](pruebas.md)). Lo que cambia:

- No pierde memoria, no termina el programa ante un error (lanza excepciones) y no revienta
  cuando el genético no encuentra ninguna regla en la primera vuelta.
- Valida los datos antes de empezar: etiquetas `a <= b <= c <= d` y valores que se adapten a
  alguna etiqueta de su atributo.
- No toca la salida estándar salvo que se le pida, y lo que escribe va por niveles.
- Da también las reglas discriminantes, el peso de cada regla y el tamaño del episodio como
  opción, y clasifica.
- Puede comparar atributos. El original tenía preparado, y apagado, algo más amplio: atributos
  nuevos con funciones de pares de atributos de la misma unidad (comparaciones, operaciones
  aritméticas y lógicas), elegidos por su información y con su sitio en el genético. De eso, INSLV
  recupera la comparación, como atributos normales.
