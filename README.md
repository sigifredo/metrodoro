# Metrodoro

Metrónomo de escritorio con un **modo práctica tipo pomodoro**: alterna rondas de práctica, con el metrónomo sonando, y descansos en silencio. Está hecho con C++23 y Qt 6.

## Funciones

- Metrónomo de 20 a 300 BPM con deslizador, botones −1/+1 y −5/+5 con repetición al mantener, cifra editable con un clic, botón TAP (promedia los últimos toques) y nombre del tempo en italiano.
- Compás de 1 a 16 tiempos con unidad 2, 4, 8 o 16 y atajos de compases habituales (2/4, 3/4, 4/4, 5/4, 6/8, 7/8, 9/8, 12/8) con los acentos agrupados.
- Acento por tiempo en cuatro niveles (fuerte, medio, débil, silencio): se cambia pulsando cada punto del compás.
- Subdivisiones: negras, corcheas, tresillos, semicorcheas, quintillos, seisillos, shuffle y dos galopes, con volumen propio.
- Seis sonidos sintetizados muestra a muestra, sin archivos de audio: Clásico, Madera, Clic, Bip, Hi-hat y Cencerro.
- Cuenta previa de 0, 1, 2 o 4 compases.
- Entrenador de velocidad: sube (o baja) X BPM cada N compases hasta una meta.
- Compases mudos: suena N compases y calla M, también al azar. Los puntos siguen marcando los tiempos, atenuados.
- Péndulo animado y destello de pantalla opcionales, sincronizados con lo que se oye.
- Preajustes con nombre y repertorios (listas de preajustes con compases por entrada y avance automático).
- Estadísticas de constancia: minutos con el metrónomo por día, últimos siete días y racha.
- Recuerda el último estado del metrónomo y de la sesión de práctica.
- Interfaz en español, inglés y francés según el idioma del sistema.

## Interfaz y atajos

La ventana tiene dos columnas. A la izquierda va lo que se toca mientras se estudia: tempo, deslizador, tiempos del compás, TAP y play. A la derecha, en pestañas, lo que se cambia poco: Metrónomo (compás, subdivisión, sonido), Práctica (sesión, entrenador, compases mudos y constancia), Biblioteca y Ajustes.

| Atajo | Acción |
|---|---|
| Espacio | Tocar o pausar |
| T | TAP |
| Ctrl+↑ / Ctrl+↓ | Tempo ±1 |
| Ctrl+Mayús+↑ / ↓ | Tempo ±5 |
| Ctrl+S | Guardar como preajuste |
| Ctrl+1 a 4 | Cambiar de pestaña |

## Modo práctica

Marca "MODO PRÁCTICA", elige un perfil y pulsa play. La sesión pasa sola de práctica a descanso y de descanso a la siguiente ronda, y avisa con un tono propio en cada cambio (ascendente al empezar a practicar, descendente al descansar y de tres notas al terminar). Termina tras la última ronda de práctica, sin descanso final. Pausar detiene el reloj y el metrónomo. Cambiar cualquier valor a mitad de sesión la reinicia.

| Perfil | Práctica | Descanso | Rondas | Total | Cuándo usarlo |
|---|---|---|---|---|---|
| Sprint | 5 min | 1 min | 6 | 35 min | Un pasaje difícil, subir velocidad, días de poca energía |
| Ligero | 10 min | 2 min | 2 | 22 min | Calentamiento o poco tiempo |
| Estándar | 15 min | 3 min | 3 | 51 min | Sesión diaria normal |
| Intenso | 25 min | 5 min | 3 | 85 min | Sesión larga cuando ya hay resistencia |

También hay un perfil Personalizado: al cambiar práctica, descanso o rondas, el selector pasa a "Personalizado" solo.

Los tiempos son puntos de partida razonables, no valores demostrados como óptimos. Se basan en que la atención sostenida en práctica deliberada ronda los 15 a 25 minutos por bloque, en que los músicos de élite estudiados por Ericsson practicaban en sesiones de hasta unos 60 a 90 minutos con pausas, y en que el pomodoro clásico es 25/5.

## Compilar y ejecutar

Requisitos: CMake 3.16 o superior, un compilador con C++23 y Qt 6.8 o superior con los módulos Core, Gui, Widgets, Multimedia y Svg.

```bash
cmake -S . -B build
cmake --build build
open build/bin/metrodoro.app
```

En Linux y Windows el ejecutable queda en `build/bin/metrodoro`. Si Qt no está en una ruta estándar, pasa `-DQt6_DIR=<ruta a lib/cmake/Qt6>` al configurar.

## Estructura

- `config/`: opciones de compilación y dependencias de CMake.
- `modules/metronome/include` y `src`: código de la aplicación.
- `modules/metronome/resources`: iconos, hoja de estilos `style.qss` y fuentes.

## Fuentes y licencias

La interfaz usa [Inter](https://github.com/rsms/inter) para el texto y [Hack](https://github.com/source-foundry/Hack) para las cifras. Se incluyen en `modules/metronome/resources/fonts` con sus licencias (`Inter-LICENSE.txt` y `Hack-LICENSE.md`).

## Ideas pendientes

- Regenerar los iconos `.icns` y `.ico` con el color de acento azul.
- Polirritmos, compases de tamaño mixto (5/8 + 7/8) e importar y exportar preajustes.
