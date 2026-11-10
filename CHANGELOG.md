# Changelog

Todos los cambios relevantes de Metrodoro se documentan en este archivo. El formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/) y el proyecto usa [versionado semántico](https://semver.org/lang/es/).

## [0.2.2] - 2026-11-09

### Corregido

- Mientras suena el metrónomo o corre la sesión de práctica, la pantalla ya no se apaga ni aparece el protector de pantalla en macOS (aserción de pantalla de IOKit) y Windows (`ES_DISPLAY_REQUIRED`).

### Cambiado

- La versión pasa de 0.2.1 a 0.2.2.

## [0.2.1] - 2026-11-09

### Corregido

- El bloqueo de suspensión mientras suena el metrónomo o corre la sesión de práctica solo funcionaba en Linux; ahora también funciona en macOS (IOKit) y Windows (`SetThreadExecutionState`).
- Los atajos de teclado se muestran con «Cmd» en lugar de «Ctrl» en macOS (Ajustes, Biblioteca y deslizador de tempo).

### Cambiado

- La versión pasa de 0.2.0 a 0.2.1.

## [0.2.0] - 2026-11-09

### Añadido

- Compases de tamaño mixto con grupos de tiempos personalizados.
- Polirritmos de dos capas.
- Importar y exportar la biblioteca (preajustes y repertorios) como archivo JSON.
- Equivalentes de escritorio de las funciones móviles: icono de bandeja del sistema, pausa al perder la salida de audio y bloqueo de suspensión en Linux.
- Plantilla `Info.plist` propia para el bundle de macOS, con versión, categoría, idiomas, versión mínima del sistema (12.0) y soporte de pantallas de alta resolución.

### Cambiado

- El identificador del bundle de macOS pasa a `dev.sigifredo.metrodoro` y el nombre visible a «Metrodoro».

## [0.1.0]

### Añadido

- Metrónomo de escritorio en C++23 y Qt 6 con modo práctica tipo pomodoro y perfiles.
- Motor de audio, acentos, subdivisiones, entrenador de tempo, compases mudos y TAP, portados de la versión Android.
- Estadísticas de constancia, aviso sonoro de cambio de fase y pestaña de Ajustes.
- Biblioteca con preajustes y repertorios.
- Fuentes Inter y Hack, y tema oscuro con acento azul.
