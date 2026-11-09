# Changelog

Todos los cambios relevantes de Metrodoro se documentan en este archivo. El formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/) y el proyecto usa [versionado semántico](https://semver.org/lang/es/).

## [0.2.0] - 2026-11-09

### Añadido

- Compases de tamaño mixto con grupos de tiempos personalizados.
- Polirritmos de dos capas.
- Importar y exportar la biblioteca (preajustes y repertorios) como archivo JSON.
- Equivalentes de escritorio de las funciones móviles: icono de bandeja del sistema, pausa al perder la salida de audio y bloqueo de suspensión en Linux.
- Plantilla `Info.plist` propia para el bundle de macOS, con versión, categoría, idiomas, versión mínima del sistema (12.0) y soporte de pantallas de alta resolución.

### Cambiado

- El identificador del bundle de macOS pasa a `org.metrodoro.metrodoro` y el nombre visible a «Metrodoro».
- La versión pasa de 0.1.0 a 0.2.0.

## [0.1.0]

### Añadido

- Metrónomo de escritorio en C++23 y Qt 6 con modo práctica tipo pomodoro y perfiles.
- Motor de audio, acentos, subdivisiones, entrenador de tempo, compases mudos y TAP, portados de la versión Android.
- Estadísticas de constancia, aviso sonoro de cambio de fase y pestaña de Ajustes.
- Biblioteca con preajustes y repertorios.
- Fuentes Inter y Hack, y tema oscuro con acento azul.
