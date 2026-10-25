# CLAUDE.md

Metrónomo de escritorio en C++23 / Qt 6.8+ (Widgets, Multimedia, Svg). Port del metrónomo web; el audio se sintetiza, no hay samples. Ver `README.md` para funciones y arquitectura.

## Comandos

```bash
cmake -S . -B build          # configurar (Debug por defecto)
cmake --build build          # compilar
./build/bin/metronome        # ejecutar
```

No hay suite de pruebas ni linter configurados. `build/` y `tmp/` están en `.gitignore`.

## Estructura

- `config/build.cmake`: estándar C++23, AUTOMOC/AUTOUIC/AUTORCC, flags por compilador, directorios de salida (`build/bin`, `build/lib`).
- `config/deps.cmake`: `find_package(Qt6 ...)` y `qt_standard_project_setup(REQUIRES 6.8)`.
- `modules/metronome/CMakeLists.txt`: **listas explícitas** `HDRS` y `SRCS`. Al agregar una clase hay que añadirla ahí, y los recursos nuevos a `qt_add_resources`.
- Prefijo de recursos Qt: `:/metronome/...` (por ejemplo `:/metronome/icons/metronome.svg`).

## Arquitectura y restricciones

- `ClickGenerator::readData` se ejecuta en el hilo de audio. Los parámetros compartidos son `std::atomic`; el estado interno de síntesis (`noteIndex_`, `samplesUntilNext_`, etc.) pertenece solo a ese hilo. Cualquier parámetro nuevo que lea `readData` debe ser atómico. No asignar memoria ni bloquear dentro de `readData`.
- La señal `beat` se emite desde el hilo de audio con `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`. Mantener ese patrón.
- `MetronomeEngine` concentra el estado y no conoce widgets. La UI solo se conecta por señales/slots; no meter lógica de tempo en `MainWindow`.
- Cambiar compás o subdivisión con el metrónomo activo reinicia la secuencia (`restartIfRunning`).
- Los rangos de BPM difieren por origen: deslizador 20 a 300, botones 10 a 260, TAP 20 a 260. Son constantes en el espacio anónimo de `MetronomeEngine.cpp` y en `kSliderMin/Max`; `setBpm` recorta a 10 a 300. No unificarlos sin confirmarlo, reproducen el comportamiento del original web.
- Textos de la UI localizados a mano en `stringsForLocale()` (`MainWindow.cpp`), sin `tr()` ni archivos `.ts`.

## Convenciones

Aplican las convenciones C++ globales del usuario (CamelCase, `.hpp`, guardas `#ifndef/#define/#endif`, strings con comillas dobles, Doxygen, orden alfabético de miembros, `.cpp` en el mismo orden que el `.hpp`).

El código existente no las cumple del todo: comentarios en prosa o `///` en lugar de Doxygen `@brief`, miembros no alfabetizados, métodos triviales definidos en línea en la cabecera. Al modificar un archivo, el código nuevo sigue la convención; no reordenar ni reformatear código ajeno al cambio sin pedirlo. Estilo local a respetar: llaves en línea propia (Allman), sangría de 4 espacios, bloques `// own`, `// qt`, `// std` en los includes, constantes `kNombre` en espacio anónimo, miembros con sufijo `_`.

## Documentación y comentarios

Comentarios y textos de documentación en español.

## Control de versiones

No hacer `git commit` ni `git push` sin petición explícita en el turno.
