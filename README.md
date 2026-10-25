# Metrodoro

Metrónomo de escritorio con un **modo práctica tipo pomodoro**: alterna rondas de práctica, con el metrónomo sonando, y descansos en silencio. Está hecho con C++23 y Qt 6.

## Funciones

- Metrónomo de 10 a 300 BPM con deslizador, botones − y + con repetición, y botón TAP para marcar el tempo con el ritmo.
- Nombre del tempo en italiano (Largo, Adagio, Allegro…) según el BPM.
- Compás de 2 a 12 tiempos, con indicador de tiempo actual y acento opcional en el primer tiempo.
- Subdivisiones: negras, corcheas, semicorcheas, tresillos y shuffle.
- Sonido sintetizado muestra a muestra, sin archivos de audio.
- Interfaz en español, inglés y francés según el idioma del sistema.

## Modo práctica

Marca "MODO PRÁCTICA", elige un perfil y pulsa play. La sesión pasa sola de práctica a descanso y de descanso a la siguiente ronda, y avisa con un beep del sistema en cada cambio. Termina tras la última ronda de práctica, sin descanso final. Pausar detiene el reloj y el metrónomo. Cambiar cualquier valor a mitad de sesión la reinicia.

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

- Recordar el último perfil y los valores personalizados entre sesiones.
- Un sonido propio para el cambio de fase en lugar del beep del sistema.
- Regenerar los iconos `.icns` y `.ico` con el color de acento azul.
