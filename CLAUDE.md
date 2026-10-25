# Metrodoro

Aplicación de escritorio en C++23 y Qt 6.8 (Widgets, Multimedia, Svg). Es un metrónomo con un modo práctica tipo pomodoro. El README describe las funciones y los perfiles; este archivo recoge lo que hay que saber para tocar el código.

## Compilar y ejecutar

```bash
cmake --build build
open build/bin/metrodoro.app
```

Si hace falta reconfigurar (por ejemplo tras añadir un archivo o un recurso): `cmake -S . -B build`. Qt está en `/opt/homebrew/lib/cmake/Qt6`. Los archivos nuevos hay que listarlos a mano en `modules/metronome/CMakeLists.txt` (`HDRS`, `SRCS` y los `FILES` de `qt_add_resources`). No hay tests automáticos.

## Arquitectura

Todo vive en `modules/metronome` (la carpeta y el prefijo de recursos `/metronome` conservan el nombre anterior; el ejecutable y el proyecto se llaman `metrodoro`). La capa de lógica no depende de widgets y la interfaz solo la observa.

- `ClickGenerator`: `QIODevice` que sintetiza el audio PCM Int16 mono a 44100 Hz. Sus parámetros son atómicos porque `readData` corre en el hilo de audio.
- `MetronomeEngine`: estado del metrónomo (BPM, compás, subdivisión, TAP) y control de reproducción. `playPause()` arranca, pausa o reanuda; `stop()` reinicia.
- `PracticeSession`: sesión pomodoro. Controla el motor solo con `stop()` y `playPause()`, y mide el tiempo contra un `QElapsedTimer` (no cuenta ticks) para no desfasarse. Emite `changed`, `runningChanged` y `transitioned`.
- `MainWindow`: arma toda la interfaz en `buildUi()` y la refresca en `updateBpm()` y `updatePractice()`.
- `PlayButton`, `RepeatButton`, `BeatDotsWidget`, `NoteSelectorWidget`: widgets pintados a mano.

Detalles que conviene no romper:

- El botón play sigue al motor (`playingChanged`) cuando el modo práctica está apagado y a la sesión (`runningChanged`) cuando está encendido. Durante un descanso el motor no suena pero la sesión sí corre, por eso el botón no puede seguir siempre al motor.
- Cambiar práctica, descanso, rondas o perfil con la sesión en marcha la detiene y la reinicia.
- `restartIfRunning()` del motor hace `stop()` y `play()`; no debe llamarse durante un descanso.
- Todo lo que depende de que el modo práctica esté activo está dentro de `practiceBody_` para atenuarlo junto.

## Estilo de código

- Cabeceras en `include/`, implementaciones en `src/`, un guion bajo final en los miembros (`engine_`).
- Los `#include` se agrupan con comentarios `// own`, `// qt` y `// std`, en ese orden.
- Comentarios y textos en español. Hay un `switch` por idioma en `stringsForLocale()` de `MainWindow.cpp` con español, inglés y francés; cualquier texto nuevo de la interfaz debe agregarse en los tres.
- Los colores y tamaños van en `resources/style/style.qss`, no en el código, salvo en los widgets pintados a mano (`PlayButton`, `RepeatButton`).

## Estilo visual

- Fondo `#202020`, texto `#F9F9F9`, gris de controles `#889299`, acento azul `#2196F3` (degradado `#64B5F6` a `#1976D2` en el icono SVG) y verde `#6FCF97` para el descanso.
- Inter para el texto y Hack para las cifras (BPM, reloj, valores, total). Las fuentes se cargan en `main.cpp` desde recursos Qt. Hack solo tiene pesos 400 y 700.
- Los iconos `.icns` y `.ico` son imágenes y siguen en el color anterior hasta que se regeneren.

## Notas de trabajo

- `tmp/` está ignorado por git y contiene la versión web original (`tmp/web`) de la que se portó la lógica del metrónomo, además de las capturas de revisión.
- En macOS, `screencapture` a pantalla completa no incluye las ventanas de otras apps. Para capturar la app hay que obtener el id de su ventana con `CGWindowListCopyWindowInfo` y usar `screencapture -x -o -l <id> archivo.png`.
- Para probar una sesión sin esperar minutos, se puede compilar una copia aparte (fuera del repositorio) donde `* 60` sea `* 2` en `PracticeSession.cpp`.