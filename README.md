# Metrónomo

Metrónomo de escritorio multiplataforma en C++23 y Qt 6. Es un port del metrónomo web (directiva `metronome`): el audio se sintetiza muestra a muestra y no se usan archivos de sonido.

## Funciones

- Tempo de 20 a 300 BPM mediante deslizador, botones `−`/`+` con autorrepetición acelerada (los botones operan entre 10 y 260 BPM) y TAP tempo.
- Nombre de tempo en italiano (Larghissimo a Prestissimo). Los rangos se solapan, por lo que un BPM puede mostrar más de un nombre.
- Compás de 2 a 12 tiempos con indicador visual del tiempo actual.
- Subdivisión en negras, corcheas, semicorcheas, tresillos y shuffle.
- Acentuación opcional del primer tiempo.
- Temporizador de cuenta regresiva que detiene el metrónomo al llegar a cero.
- Interfaz en español, inglés o francés según la configuración regional (francés por defecto).

## Requisitos

- CMake 3.16 o superior
- Compilador con soporte de C++23
- Qt 6.8 o superior con los módulos Core, Gui, Widgets, Multimedia y Svg

En Debian:

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev qt6-svg-dev
```

Verificar que la versión de Qt empaquetada sea 6.8 o superior; `qt_standard_project_setup(REQUIRES 6.8)` aborta la configuración en caso contrario.

## Compilación y ejecución

```bash
cmake -S . -B build
cmake --build build
./build/bin/metronome
```

Sin `CMAKE_BUILD_TYPE` explícito se compila en `Debug` (`-g -Wall -Wextra`). Para optimizar: `-DCMAKE_BUILD_TYPE=Release`.

## Estructura

```
config/                    Opciones de compilación (build.cmake) y dependencias (deps.cmake)
modules/metronome/
  include/                 Cabeceras .hpp
  src/                     Implementaciones .cpp y main.cpp
  resources/               Iconos, SVG y hoja de estilos QSS (embebidos con qt_add_resources)
```

## Arquitectura

| Clase                                                                | Responsabilidad                                                                                                                                         |
| -------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ClickGenerator`                                                     | `QIODevice` secuencial que genera PCM Int16 mono a 44100 Hz. Cada nota es un seno con envolvente lineal de 10 ms. `readData` corre en el hilo de audio. |
| `MetronomeEngine`                                                    | Estado y reglas (BPM, compás, subdivisión, TAP, temporizador) y control del `QAudioSink`. No depende de widgets.                                        |
| `MainWindow`                                                         | Construye la interfaz y conecta señales del motor con los widgets.                                                                                      |
| `BeatDotsWidget`, `NoteSelectorWidget`, `PlayButton`, `RepeatButton` | Widgets de presentación.                                                                                                                                |

## Licencia

Sin definir.
