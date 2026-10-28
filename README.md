# VehicleSim

Simulación 2D de tráfico urbano con física de vehículos, IA de dirección y
comportamientos de enjambre vectorizados con SIMD.

Proyecto del curso de Programación Avanzada · Cinvestav · 2026/1.

> **Estado: en construcción.** Sprint 0A (fundaciones). El proyecto todavía no
> compila; es intencional mientras se construye el sistema de build.

## Descripción

Un clon top-down de conducción estilo GTA 1, enfocado en tres cosas:

1. **Física de vehículos** con modelo de neumático asimétrico (agarre lateral
   alto, resistencia de rodadura baja) y derrape controlable con freno de mano.
2. **Dirección autónoma**: vehículos que siguen rutas por una ciudad en grilla
   y peatones que se mueven como enjambre.
3. **Rendimiento**: cálculo vectorial del enjambre con backends escalar, SSE2,
   AVX2 y NEON, seleccionables en tiempo de ejecución.


## Stack

| Pieza | Tecnología |
|---|---|
| Lenguaje | C++20 |
| Gráficos | OpenGL 3.3 core + glad |
| Ventana y entrada | GLFW 3.4 (teclado y gamepad) |
| Física | Box2D v3 |
| Interfaz y depuración | Dear ImGui |
| Build | CMake 3.28+ con presets |
| Tests | doctest |
| Empaquetado | CPack (DEB, AppImage, NSIS, DMG) |

## Requisitos

- CMake 3.28 o superior
- Compilador con C++20: GCC 12+, Clang 15+ o MSVC 19.3+
- Git
- Conexión a internet en el primer configure (las dependencias se descargan
  con `FetchContent` y después quedan cacheadas)

## Compilación

Pendiente de documentar en el Sprint 0.3. Cuando exista el sistema de build
habrá tres presets:

```bash
cmake --preset dev-debug
cmake --build --preset dev-debug
```

## Estructura del repositorio

```
.
├── assets/        Texturas, sprites y mapas
├── docs/          Documentación y bitácora del proyecto
├── src/           Código fuente
├── tests/         Pruebas automatizadas
├── third_party/   Código de terceros vendorizado
└── .github/       Integración continua y empaquetado
```

Las carpetas de código aparecen a medida que avanza cada sprint.

## Documentación

- [Índice de documentación](docs/README.md)
- [Visión y alcance](docs/00-vision.md)
- [Hoja de ruta por sprints](docs/sprints/roadmap.md)
- [Fundamentos de física](docs/teoria/fisica.md)

## Hoja de ruta

| Sprint | Contenido | Estado |
|---|---|---|
| 0A | Repo, CMake, ventana, entrada, game loop | En curso |
| 0B | Matemáticas, renderer, ImGui, CI, empaquetado | Pendiente |
| 1 | Box2D, jerarquía de vehículos, mapa, filtros | Pendiente |
| 2 | Steering y modelo de neumáticos | Pendiente |
| 3 | Rutas con STL, A*, minimapa | Pendiente |
| 4 | Flocking SIMD con backends múltiples | Pendiente |
| 5 | Joints y máquinas de estado con bits | Pendiente |
| 6 | Algoritmos evolutivos en modo headless | Pendiente |
| 7 | Release y distribución | Pendiente |

## Licencia

MIT. Ver [LICENSE](LICENSE).

## Créditos

- Formulación de dirección: Craig Reynolds y *Nature of Code* (Daniel Shiffman).
- Referencias de conducción 2D: [patkerr.co.uk/2d-vehicles](https://patkerr.co.uk/2d-vehicles/).
- Bibliotecas: [Box2D](https://github.com/erincatto/box2d),
  [GLFW](https://www.glfw.org/), [Dear ImGui](https://github.com/ocornut/imgui),
  [doctest](https://github.com/doctest/doctest).
