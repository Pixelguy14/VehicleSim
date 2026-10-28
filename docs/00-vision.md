# Visión y alcance

## Qué es

Un simulador 2D top-down de tráfico urbano escrito en C++20, con física de
vehículos personalizada sobre Box2D, dirección autónoma, navegación por rutas
y comportamientos de enjambre calculados con SIMD.

## Qué no es

No es un videojuego completo ni un clon de GTA. Quedan fuera del alcance:
misiones, narrativa, audio, multijugador, guardado de partida, editor de mapas
dentro del juego y compilación para móviles.

## Objetivos de aprendizaje

1. Diseño orientado a objetos con jerarquía de vehículos y polimorfismo real.
2. Uso de la STL en algoritmos de búsqueda (Dijkstra y A* con colas de
   prioridad) y en estructuras de datos para rutas.
3. Programación vectorial con intrínsecos, con backend escalar de referencia y
   comparación numérica entre backends.
4. Operaciones bitwise propias, no solo las de la biblioteca de física.
5. Algoritmos evolutivos aplicados a la calibración de parámetros, con
   simulación sin renderizado para iterar rápido.
6. Ingeniería de software: build reproducible, tests, CI en tres sistemas
   operativos y empaquetado instalable.

## Alcance del MVP

Clasificación MoSCoW. Si el calendario aprieta, se recorta de abajo hacia
arriba y nunca al revés.

### Must

- InputManager con mapa de acciones lógicas y detección de flancos.
- Game loop con paso fijo, snapshot e interpolación de renderizado.
- Vec2, transformadas y cámara 2D con tests.
- Panel de Dear ImGui con métricas de frame y de física.
- Tilemap en grilla con fusión de colisiones (greedy meshing).
- Minimapa con textura horneada.
- Steering, modelo de neumático y freno de mano.
- Rutas con A* y seguimiento de camino.
- SIMD con backends escalar, SSE2 y AVX2.
- Algoritmos evolutivos en modo headless.
- Instaladores para Linux, Windows y macOS.

### Should

- Soporte de gamepad.
- Selector de backend SIMD en vivo.
- Sliders de ajuste de parámetros en tiempo real.
- Binario universal de macOS.
- CI con sanitizers.

### Could

- Minimapa rotado con framebuffer.
- Reasignación de controles desde la interfaz.
- Reproducción de simulaciones grabadas.
- Comparativa medida entre Dijkstra y A*.

### Won't

- Multijugador, audio, guardado, editor in-game, Flatpak, AUR, móvil.

## Criterio de éxito

El proyecto está terminado cuando una persona puede descargar un instalador de
su sistema operativo, ejecutarlo sin instalar dependencias, ver una ciudad con
vehículos y peatones moviéndose a 60 FPS, ajustar parámetros en vivo y observar
cómo la configuración evolucionada mejora el comportamiento del enjambre.
