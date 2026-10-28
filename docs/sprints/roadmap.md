/VehicleSim
│── /assets          (Texturas/Sprites para autos, buses, personas)
│── /src
│   ├── main.cpp
│   ├── /core
│   │   ├── Game.h/cpp       (Game Loop con Fixed Time Step)
│   │   └── Window.h/cpp     (GLFW/OpenGL context)
│   ├── /math
│   │   ├── Vec2.h           (Matemáticas básicas)
│   │   ├── SimdMath.h/cpp   (Wrappers de Intrinsics AVX/SSE)
│   │   └── CollisionMasks.h (Definición de bits para colisiones)
│   ├── /entities
│   │   ├── Vehiculo.h       (Clase Abstracta Base)
│   │   ├── Autobus.h/cpp    (Derivada)
│   │   ├── Moto.h/cpp       (Derivada)
│   │   └── Persona.h/cpp    (Derivada)
│   ├── /ai
│   │   ├── Steering.h/cpp   (Desired Velocity y Steering Force)
│   │   ├── PathFollowing.h/cpp (Uso de STL)
│   │   └── FlockingSIMD.h/cpp  (Fuerzas grupales en ALU vectorial)
│   ├── /physics
│   │   ├── PhysicsWorld.h/cpp  (Integración con Box2D)
│   │   └── BitwiseFilter.h/cpp (Manejo de colisiones con & y >>)
│   ├── /evolution          (Opcional - Tarea de Evolutivos)
│   │   ├── GeneticAlgorithm.h/cpp
│   │   └── Fitness.h/cpp
│   └── /graphics
│       └── Renderer.h/cpp   (Dibujado OpenGL)


Sprints (Agile Methodology) C++ and OpenGL

El proyecto tiene un nivel de dificultad medio-alto debido a la mezcla de tecnologías. Dividámoslo en 6 Sprints:
Sprint 1: Cimientos, POO y Box2D (El Esqueleto)

    Objetivo: Integrar OpenGL, Box2D y crear la jerarquía de clases base.

    Tareas:

        Configurar Box2D en el proyecto. Crear el mundo físico con gravedad.

        Implementar la clase abstracta Vehiculo con atributos (b2Body* body, masa, velocidad máxima) y métodos virtuales puros (GetDesiredVelocity(), GetSteeringForce()).

        Crear clases derivadas (Autobus, Moto, Persona) con diferentes formas geométricas en Box2D y parámetros de masa/inercia.

        Implementar la máscara de bits inicial (ej. CATEGORY_VEHICLE = 0x0001, CATEGORY_PERSON = 0x0002) y asignarlas a los b2FixtureDef de Box2D para filtrar colisiones de forma nativa y rápida.

    Entregable: Vehículos de diferentes formas cayendo y chocando contra el suelo, sin atravesarse entre sí gracias a los bits.

Sprint 2: El Motor de Dirección (Steering Behaviors)

    Objetivo: Lograr que los vehículos se muevan solos hacia un objetivo.

    Tareas:

        Implementar las fórmulas de Craig Reynolds: Desired Velocity = normalize(Target - Position) * MaxSpeed.

        Steering Force = Desired Velocity - Current Velocity.

        Aplicar esta fuerza al cuerpo de Box2D usando body->ApplyForce() o body->ApplyLinearImpulse().

        Hacer que las clases derivadas modifiquen su comportamiento (ej. la Moto tiene alta aceleración y bajo radio de giro; el Autobus es lento y pesado).

        Polimorfismo en acción: Un std::vector<Vehiculo*> actualizando cada frame llamando a GetSteeringForce().

    Entregable: Los vehículos persiguen un punto que se mueve con el mouse.

Sprint 3: Path Following con STL

    Objetivo: Implementar navegación por rutas usando estructuras de datos de la STL.

    Tareas:

        Crear un grafo de nodos (rutas o intersecciones).

        Usar std::priority_queue (como en Huffman) para implementar un algoritmo de búsqueda (Dijkstra o A*) y encontrar el camino más corto entre el vehículo y su destino final.

        Alternativa/Secundaria: Usar std::vector junto a std::sort (o qsort si el profesor exige C puro) para encontrar el nodo de ruta más cercano al vehículo iterando y evaluando distancias.

        Integrar el "Path Following" (Cap 6.8 de Nature of Code): el vehículo busca el punto más cercano en la ruta y aplica fuerzas de dirección hacia adelante.

    Entregable: Los vehículos calculan y siguen rutas predefinidas por la ciudad evitando obstáculos estáticos.

Sprint 4: Group Behaviors con SIMD (El Desafío)

    Objetivo: Simular cientos de personas/vehículos usando Intrinsics (AVX/MMX).

    Tareas:

        Refactorización crítica: Extraer las posiciones y velocidades de los Vehiculo a estructuras planas (SoA): float posX[1000], float posY[1000].

        Implementar las fuerzas de Flocking (Separación, Alineación, Cohesión) usando Intrinsics (_mm256_load_ps, _mm256_sub_ps, _mm256_add_ps).

        Procesar de a 8 vehículos (AVX) o 4 vehículos (SSE) a la vez para calcular las fuerzas de repulsión entre ellos.

        Inyectar el resultado del cálculo SIMD de vuelta en los cuerpos de Box2D.

    Entregable: Una multitud (crowd) de peatones moviéndose como un enjambre a 60 FPS estables, demostrando el uso de la ALU vectorial.

Sprint 5: Motores Físicos y Optimización Bitwise

    Objetivo: Usar Box2D Joints y optimizar la lógica del juego con bits.

    Tareas:

        Usar b2RevoluteJoint o b2PrismaticJoint para simular algo visual (ej. que un autobús tenga un remolque, o que las ruedas de una moto giren visualmente).

        Usar operaciones bitwise (>>, &) en tu propia lógica de juego (no solo en Box2D). Por ejemplo, para saber si un vehículo está "Herido", "EnRuta" y "EsAutobus" todo en un solo uint8_t de estado usando máscaras.

        Implementar "Collision Events" de Box2D: Cuando un vehículo choca con una persona (usando los bits para saber qué chocó), la persona cambia su estado y su comportamiento.

    Entregable: Vehículos con partes móviles unidas por joints y un sistema de colisiones con respuestas lógicas rápidas basadas en bits.

Sprint 6: Algoritmos Evolutivos (Integración de tu tarea)

    Objetivo: Usar tu código de evolutivos para "entrenar" o calibrar la simulación.

    Tareas:

        En lugar de evolucionar rutas, evoluciona los pesos (weights) de las fuerzas de Steering. Por ejemplo, ¿cuál es la mezcla perfecta de Separación y Cohesión para que las personas no se choquen pero se muevan en grupo?

        Función de Fitness: Evaluar qué tan lejos llegaron los vehículos sin chocar en X segundos.

        Aplicar cruce (crossover) empaquetando los pesos en bits y aplicando máscaras binarias.

        (Bonus) Correr la simulación sin renderizar (Headless) para calcular generaciones de evolución rápido, y luego mostrar los resultados en OpenGL.

    Entregable: Un sistema que ajusta automáticamente los parámetros de comportamiento de los vehículos para encontrar la configuración óptima.

💡 Consejos Técnicos cruciales para tu éxito:

    Sobre SIMD: No intentes hacer que los punteros de Vehiculo usen SIMD directamente. Usa una clase System o Manager (ej. FlockingSystem) que reciba un std::vector<float> con las posiciones empaquetadas. En C++ moderno, puedes usar alignas(32) para asegurar que tus arrays flotantes estén alineados para AVX.

    Sobre Box2D: Box2D ya usa máscaras de bits internamente para las colisiones. Aprovéchalo usando las constantes categoryBits y maskBits en los b2Filter. Es la forma más rápida y "C++-era" de hacerlo.

    Sobre la Evolución: La evolución de algoritmos puede ser muy lenta si evalúas a cada vehículo de forma individual. Enfoca tu fitness a evaluar parámetros globales (ej. el promedio de colisiones del enjambre) o a evolucionar una red neuronal pequeña que tome decisiones de steering (esto último es un plus gigante para tu nota).

    El "Muro" de la Física: Integrar la IA (Steering) con Box2D a veces causa que los vehículos "vibren" o se volteen. Si eso pasa, limita la velocidad angular o aplica las fuerzas al centro de masa (ApplyForceToCenter) en lugar de a un punto específico.