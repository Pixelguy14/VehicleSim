// main.cpp ya no conoce GLFW: la ventana y la entrada estan encapsuladas en
// core/Window y input/InputManager. Solo necesita glad para las llamadas de
// OpenGL y Box2D para la prueba de humo.
#include <cstdio>
#include <cstring>
#include <exception>

#include <box2d/box2d.h>
#include <glad/gl.h>

#include "core/Version.h"
#include "core/Window.h"
#include "input/InputManager.h"

namespace {

constexpr const char* kUsage =
    "Uso: vehicle_sim [opciones]\n"
    "\n"
    "Sin argumentos abre la ventana.\n"
    "\n"
    "Opciones:\n"
    "  --version   muestra la version y termina\n"
    "  --selftest  comprueba que las dependencias enlazan y funcionan\n"
    "  --help      muestra esta ayuda y termina\n";

void PrintVersion() {
    std::printf("%s %s\n", vs::kProjectName, vs::kProjectVersion);
}

void PrintBanner() {
    std::printf("%s %s\n", vs::kProjectName, vs::kProjectVersion);
    std::printf("Simulacion 2D de trafico urbano\n");
    std::printf("Compilado con %s %s, configuracion %s\n", VS_COMPILER_ID, VS_COMPILER_VERSION,
                VS_BUILD_CONFIG);
    std::printf("\n");
    // Sin esto, al redirigir la salida a un archivo el banner (stdout, con
    // buffer) aparece despues de los errores (stderr, sin buffer), y el log se
    // lee en un orden que no ocurrio.
    std::fflush(stdout);
}

// Deja caer un circulo sobre un suelo estatico y comprueba que queda encima.
// No es un test unitario (eso llega con doctest en el Sprint 0.8): es una
// prueba de humo que demuestra que las tres dependencias estan enlazadas y que
// Box2D simula de verdad, no solo compila.
int RunSelfTest() {
    std::printf("--- dependencias ---\n");
    std::printf("GLFW     %s\n", vs::GlfwVersion());

    const b2Version box2d = b2GetVersion();
    std::printf("Box2D    %d.%d.%d\n", box2d.major, box2d.minor, box2d.revision);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, -10.0f};
    const b2WorldId worldId = b2CreateWorld(&worldDef);

    // Suelo: caja estatica de 20 x 1 con la cara superior en y = 0.
    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = b2Vec2{0.0f, -0.5f};
    const b2BodyId groundId = b2CreateBody(worldId, &groundDef);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    const b2Polygon groundBox = b2MakeBox(10.0f, 0.5f);
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // Circulo dinamico de radio 0.5 que cae desde y = 4.
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{0.0f, 4.0f};
    const b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    const b2Circle circle = {{0.0f, 0.0f}, 0.5f};
    b2CreateCircleShape(bodyId, &shapeDef, &circle);

    constexpr float kStep = 1.0f / 60.0f;
    constexpr int kSteps = 180;  // 3 segundos
    for (int i = 0; i < kSteps; ++i) {
        b2World_Step(worldId, kStep, 4);
    }

    const b2Vec2 position = b2Body_GetPosition(bodyId);
    b2DestroyWorld(worldId);

    std::printf("Circulo  y = %.3f tras 3 s (se espera ~0.500)\n",
                static_cast<double>(position.y));

    const bool ok = position.y > 0.45f && position.y < 0.55f;
    std::printf("%s\n", ok ? "OK: las dependencias funcionan"
                           : "FALLO: el circulo no se detuvo sobre el suelo");
    return ok ? 0 : 1;
}

// Bucle principal por ahora vacio: abre la ventana, limpia el fondo y cierra
// con Escape. El game loop con paso fijo y el renderer llegan en 0.6 y 0.7.
int RunWindow() {
    vs::Window window;
    vs::InputManager input(window);

    bool debug = false;

    std::printf("Ventana %dx%d abierta. El renderer llega en 0.7.\n", window.FramebufferWidth(),
                window.FramebufferHeight());
    std::printf("Controles: W/S o flechas, espacio freno de mano, F1 depuracion, R reset,\n");
    std::printf("Escape salir.\n");
    std::fflush(stdout);

    while (!window.ShouldClose()) {
        window.PollEvents();

        // Una sola lectura de hardware por frame. En 0.6 este BeginStep se
        // llamara una vez por cada paso fijo que toque ejecutar, no una vez por
        // frame: ese es justo el cambio que InputState hace posible.
        input.Poll();
        input.BeginStep();

        if (input.WasPressed(vs::InputAction::Quit)) {
            window.RequestClose();
        }

        if (input.WasPressed(vs::InputAction::ToggleDebug)) {
            debug = !debug;
            std::printf("depuracion %s | giro %+.2f acelerador %.2f freno %.2f\n",
                        debug ? "ON" : "OFF", static_cast<double>(input.SteerAxis()),
                        static_cast<double>(input.ThrottleAxis()),
                        static_cast<double>(input.BrakeAxis()));
            std::fflush(stdout);
        }

        // Estos mensajes existen solo para demostrar que los flancos llegan una
        // vez por pulsacion y no una vez por frame. Desaparecen en 0.6.
        if (input.WasPressed(vs::InputAction::Throttle)) {
            std::printf("acelerador presionado\n");
            std::fflush(stdout);
        }
        if (input.WasReleased(vs::InputAction::Throttle)) {
            std::printf("acelerador soltado\n");
            std::fflush(stdout);
        }
        if (input.WasPressed(vs::InputAction::Reset)) {
            std::printf("reset\n");
            std::fflush(stdout);
        }

        glClearColor(0.09F, 0.10F, 0.12F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        window.SwapBuffers();
    }

    return 0;
}

// Devuelve el codigo de salida del proceso. 0 exito, 2 uso incorrecto: la
// convencion que siguen casi todas las herramientas de linea de comandos.
int Run(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "--version") == 0) {
            PrintVersion();
            return 0;
        }
        if (std::strcmp(arg, "--selftest") == 0) {
            return RunSelfTest();
        }
        if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
            std::printf("%s", kUsage);
            return 0;
        }
        std::fprintf(stderr, "Opcion desconocida: %s\n\n%s", arg, kUsage);
        return 2;
    }

    PrintBanner();
    return RunWindow();
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return Run(argc, argv);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Error: %s\n", error.what());
        return 1;
    }
}
