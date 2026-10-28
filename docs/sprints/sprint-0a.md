# Sprint 0A — Fundaciones

**Estado:** en curso
**Objetivo:** tener un repositorio limpio y un ejecutable que abre una ventana,
lee entrada del teclado y corre un game loop con paso fijo de tiempo.

## Sub-pasos

- [x] 0.1 Higiene de repositorio (`.gitignore`, `.gitattributes`,
      `.editorconfig`, `.clang-format`)
- [x] 0.2 Esqueleto documental, hoja de ruta y visión
- [ ] 0.3 CMake + presets + warnings por compilador
- [ ] 0.4 Dependencias: glad vendorizado, GLFW 3.4, Box2D v3, doctest
- [ ] 0.5 Ventana OpenGL 3.3 core
- [ ] 0.5b InputManager con acciones lógicas y detección de flancos
- [ ] 0.6 Game loop con acumulador, snapshot e interpolación
- [ ] 0.11 Retrospectiva y documentación del sprint

## Criterio de aceptación

El ejecutable abre una ventana de 1280x720, se cierra con Escape, muestra un
contador de pasos físicos que avanza a 60 por segundo independientemente del
framerate, y responde sin retardo perceptible cuando se mantiene pulsada una
tecla de acción.

## Decisiones tomadas

Se documentan como ADR al cerrar el sprint.

## Problemas encontrados

_(se completa al cerrar el sprint)_

## Retrospectiva

_(se completa al cerrar el sprint)_
