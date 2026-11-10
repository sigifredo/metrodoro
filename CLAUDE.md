# Metrodoro

Aplicación de escritorio en C++23 y Qt 6.8 (Widgets, Multimedia, Svg). Es un metrónomo con un modo práctica tipo pomodoro. El README describe las funciones y los perfiles; este archivo recoge lo que hay que saber para tocar el código.

## Compilar y ejecutar

```bash
cmake --build build
open build/bin/metrodoro.app   # en Linux: build/bin/metrodoro
```

Si hace falta reconfigurar (por ejemplo tras añadir un archivo o un recurso): `cmake -S . -B build`. Qt está en `/opt/homebrew/lib/cmake/Qt6`. Los archivos nuevos hay que listarlos a mano en `modules/metronome/CMakeLists.txt` (`HDRS`, `SRCS` y los `FILES` de `qt_add_resources`). No hay tests automáticos; la lógica pura (`ClickConfig`, `BarPlanner`, `TapTempo`) se puede comprobar compilándola aparte con un `main` que use `assert`.

## Arquitectura

Todo vive en `modules/metronome` (la carpeta y el prefijo de recursos `/metronome` conservan el nombre anterior; el ejecutable y el proyecto se llaman `metrodoro`). La capa de lógica no depende de widgets y la interfaz solo la observa. La lógica y el formato JSON vienen de la versión Android (`tmp/metrodoro`, Kotlin): los nombres y el comportamiento se mantienen para que ambas versiones sigan alineadas.

- `ClickConfig`: valor con todo lo que define cómo suena (BPM, compás, grupos de tiempos, acentos, subdivisión, sonido, cuenta previa, entrenador, mudos, polirritmo) y su JSON. Incluye `defaultAccents`, `beatGroups` y `tempoNames`. El hilo de audio trabaja con copias.
- `BarPlanner` y `TapTempo`: lógica pura. `BarPlanner` decide por compás la cuenta previa, el entrenador y los mudos.
- `ClickSynth`: tablas de los seis timbres (tres alturas cada uno) y el aviso de cambio de fase.
- `ClickGenerator`: `QIODevice` que renderiza PCM Int16 mono a 44100 Hz con un contador de fracción en `double` y hasta 16 voces solapadas. Toma un candado una vez por nota para leer la configuración en vivo o el repertorio. Anota cada pulso con su muestra. La segunda capa (polirritmo) se programa con su propio contador dentro del mismo bucle y emite pulsos con `poly = true`.
- `MetronomeEngine`: configuración, TAP, repetición de ±BPM, repertorio en curso (`SetlistRun`) y reproducción (`start()`, `stop()`, `toggle()`). Entrega cada pulso cuando `processedUSecs` del sink alcanza su muestra, así que puntos, péndulo y destello coinciden con el sonido. `edit()` aplica una transformación a la configuración.
- `PracticeSession`: sesión pomodoro. Controla el motor solo con `start()` y `stop()`, y mide el tiempo contra un `QElapsedTimer` (no cuenta ticks). Emite `changed`, `runningChanged` y `transitioned(Transition)`.
- `SleepInhibitor`: bloqueo de suspensión; logind por D-Bus en Linux (`METRODORO_HAS_DBUS`), aserción de IOKit en macOS y `SetThreadExecutionState` en Windows.
- `AppSettings` (preferencias y último estado), `StatsStore` (segundos por día, 90 días) y `LibraryStore` (preajustes y repertorios en JSON, con `exportJson`/`importJson`): todos con `QSettings`.
- `MainWindow`: dos columnas. `TempoPanel` a la izquierda (cifra, deslizador, `BeatRowWidget`, `PendulumWidget`, TAP, play y aviso de práctica) y un `QTabWidget` a la derecha con `MetronomeTab`, `PracticeTab`, `LibraryTab` y `SettingsTab`, cada una dentro de un `QScrollArea` (`makeScrollPage`). Aquí viven los atajos y la lógica de qué sigue el botón de play.
- Widgets pintados a mano: `PlayButton`, `RepeatButton`, `BeatRowWidget`, `PendulumWidget`, `SubdivisionSelector` (fichas con el glifo teñido o puntos de huecos) y `WeekChart`. Los pequeños reutilizables (`ChipRow`, `StepperRow`, tarjetas, `confirmDialog`) están en `Controls`.

Detalles que conviene no romper:

- El botón play está encendido si el motor suena o si la sesión corre (`updatePlayButton()` en `MainWindow`). Durante un descanso el motor calla pero la sesión sigue corriendo, por eso no puede seguir solo al motor. Con el modo práctica activo y sin repertorio, play y la barra espaciadora alternan la sesión; si no, el motor.
- Cambiar práctica, descanso, rondas o perfil con la sesión en marcha la detiene y la reinicia. Activar o desactivar el modo práctica lo detiene todo.
- Editar la configuración mientras suena un repertorio sale del repertorio y sigue con lo editado (`MetronomeEngine::setConfig`). Durante un repertorio la configuración que se ve es la de la pieza en curso y no se guarda como último estado.
- Los cambios de número de tiempos, unidad o subdivisión reinician el compás en curso; los de tempo, acentos, sonido y volumen de subdivisión se aplican en la siguiente nota. `BarPlanner::rebase` reinicia el entrenador si cambia el tempo base, y el BPM que se muestra durante el entrenador es el del pulso (`LiveState::bpm`), no `config.bpm`.
- Los compases mudos siguen mostrando los tiempos, atenuados, para no perder el sitio.
- Todo texto nuevo va en español, inglés y francés con `txt(es, en, fr)` (`Texts.hpp`), que elige según `QLocale()`.
- El motor crea el `QAudioSink` de nuevo en `start()` si cambió la salida predeterminada, y `MainWindow` pausa (no reinicia) si la salida en uso desaparece (`outputAvailable()`), como el pausar al desconectar auriculares de Android.
- El `Info.plist` del bundle de macOS sale de `modules/metronome/resources/Info.plist.in` (no de la plantilla por defecto de CMake). El identificador es `dev.sigifredo.metrodoro` y la versión sale de `project(... VERSION ...)` en el `CMakeLists.txt` raíz; al subirla, actualiza también `CHANGELOG.md`.
- Lo que solo tiene sentido en móvil no se portó: servicio en primer plano, notificación, botones de auriculares, vibración y tema claro/oscuro del sistema. El equivalente de escritorio es la pausa al perder la salida de audio y el bloqueo de suspensión.

## Estilo de código

- Cabeceras en `include/`, implementaciones en `src/`, un guion bajo final en los miembros (`engine_`).
- Los `#include` se agrupan con comentarios `// own`, `// qt` y `// std`, en ese orden.
- Comentarios y textos en español. Los textos de la interfaz están en español, inglés y francés mediante `txt()` (`Texts.hpp`); cualquier texto nuevo debe agregarse en los tres.
- Los colores y tamaños van en `resources/style/style.qss`, no en el código, salvo en los widgets pintados a mano.

## Commits

- Los mensajes siguen [Conventional Commits](https://www.conventionalcommits.org/es/v1.0.0/): `tipo(ámbito opcional): descripción`, con tipos como `feat`, `fix`, `docs`, `chore`, `refactor`, `build`, `style`, `perf` y `test`. Los cambios incompatibles llevan `!` tras el tipo o un pie `BREAKING CHANGE:`.
- La descripción va en español y en una sola línea, sin saltos internos. Sin líneas de atribución (`Co-Authored-By` y similares).

## Estilo visual

- Fondo `#202020`, texto `#F9F9F9`, gris de controles `#889299`, acento azul `#2196F3` (degradado `#64B5F6` a `#1976D2` en el icono SVG) y verde `#6FCF97` para el descanso.
- Inter para el texto y Hack para las cifras (BPM, reloj, valores, total). Las fuentes se cargan en `main.cpp` desde recursos Qt. Hack solo tiene pesos 400 y 700.
- Los iconos `.icns` y `.ico` son imágenes generadas a partir de `metronome.svg`: si cambia el SVG hay que regenerarlos.

## Notas de trabajo

- `tmp/` está ignorado por git y contiene la versión web original (`tmp/web`) de la que se portó la lógica del metrónomo, además de las capturas de revisión y de la versión Android (`tmp/metrodoro`, con `CLAUDE.md` y `docs/` propios) que sirve de referencia de comportamiento.
- Para ver la interfaz sin pantalla (o con la sesión bloqueada) se puede enlazar un `main` auxiliar, fuera del repositorio, con los `.o` de `build/` y guardar `MainWindow::grab()` con `QT_QPA_PLATFORM=offscreen`. Conviene fijar `XDG_CONFIG_HOME` a una carpeta temporal para no tocar los ajustes reales.
- En macOS, `screencapture` a pantalla completa no incluye las ventanas de otras apps. Para capturar la app hay que obtener el id de su ventana con `CGWindowListCopyWindowInfo` y usar `screencapture -x -o -l <id> archivo.png`.
- Para probar una sesión sin esperar minutos, se puede compilar una copia aparte (fuera del repositorio) donde `* 60` sea `* 2` en `PracticeSession.cpp`.
