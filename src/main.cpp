// main.cpp ya no conoce GLFW: la ventana y la entrada estan encapsuladas en
// core/Window y input/InputManager. Solo necesita glad para las llamadas de
// OpenGL y Box2D para la prueba de humo.
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <exception>

#include <box2d/box2d.h>
#include <glad/gl.h>

#include "core/FixedStepper.h"
#include "core/FrameStats.h"
#include "core/Version.h"
#include "core/Window.h"
#include "graphics/Renderer.h"
#include "input/InputManager.h"
#include "math/Angles.h"
#include "math/Camera2D.h"
#include "math/Vec2.h"

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

// Estado de la escena de demostracion. En el Sprint 2 lo sustituye el mundo de
// Box2D; existe ahora para comprobar que la simulacion avanza en pasos fijos y
// que el render interpola entre dos de ellos.
struct SceneState {
    vs::Vec2 cameraPosition{};
    float squareAngle = 0.0F;
    float barAngle = 0.0F;
    float circlePhase = 0.0F;
};

// Un paso de simulacion. Todo lo que cambia con el tiempo se modifica aqui y
// solo aqui: nada de leer el reloj dentro del render, porque entonces el estado
// dependeria del frame rate y dejaria de ser reproducible.
void StepScene(SceneState& scene, const vs::InputManager& input, float deltaSeconds) {
    float speed = 8.0F;
    if (input.IsDown(vs::InputAction::Handbrake)) {
        speed *= 3.0F;
    }

    const float distance = speed * deltaSeconds;
    scene.cameraPosition += vs::Vec2{input.SteerAxis() * distance,
                                     (input.ThrottleAxis() - input.BrakeAxis()) * distance};

    // Los angulos se acotan cada paso para que no crezcan sin limite.
    scene.squareAngle = vs::WrapAngle(scene.squareAngle + 0.9F * deltaSeconds);
    scene.barAngle = vs::WrapAngle(scene.barAngle - 0.6F * deltaSeconds);

    // Esta fase no se acota a proposito: sin() es periodica, y al interpolar
    // entre -pi y pi un salto de vuelta se veria como un rebote.
    scene.circlePhase += 2.0F * deltaSeconds;
}

// Bucle principal: paso fijo de 60 Hz, render a lo que de el monitor.
//
// El orden importa. Primero se lee el hardware una vez; despues se ejecutan los
// pasos fijos que toquen, y `input.BeginStep()` se llama una vez por paso para
// que un flanco se consuma en el primero que lo vea y no se repita en el
// segundo. El render, al final, no simula nada: solo interpola entre el estado
// anterior y el actual.
int RunWindow() {
    vs::Window window;
    vs::InputManager input(window);

    vs::Camera2D camera;
    camera.SetViewport(window.FramebufferWidth(), window.FramebufferHeight());
    camera.SetZoom(60.0F);

    vs::Renderer renderer;
    vs::FixedStepper stepper;  // 1/60 s
    vs::FrameStats stats;

    SceneState current;
    SceneState previous;

    bool showGrid = true;

    auto lastFrame = std::chrono::steady_clock::now();

    std::printf("Ventana %dx%d abierta. Paso fijo 60 Hz, titulo con FPS y coste de fisica.\n",
                window.FramebufferWidth(), window.FramebufferHeight());
    std::printf("Controles: W/S y A/D mueven la camara (temporal, hasta que exista el\n");
    std::printf("vehiculo), espacio acelera el desplazamiento, F1 oculta la grilla,\n");
    std::printf("Escape salir.\n");
    std::fflush(stdout);

    while (!window.ShouldClose()) {
        const auto frameStart = std::chrono::steady_clock::now();
        const double frameSeconds = std::chrono::duration<double>(frameStart - lastFrame).count();
        lastFrame = frameStart;

        window.PollEvents();
        input.Poll();

        // Tras una pausa larga (minimizar, cambiar de escritorio, un breakpoint)
        // no se simula el tiempo perdido: el reloj de pared no es tiempo de
        // juego. Se descarta y este frame solo dibuja.
        const bool stalled = frameSeconds > 0.25;
        if (stalled) {
            stepper.Reset();
        }

        const int stepCount = stalled ? 0 : stepper.Advance(frameSeconds);
        const float fixedDelta = static_cast<float>(stepper.FixedDelta());

        const auto physicsStart = std::chrono::steady_clock::now();
        for (int step = 0; step < stepCount; ++step) {
            input.BeginStep();

            if (input.WasPressed(vs::InputAction::Quit)) {
                window.RequestClose();
            }

            if (input.WasPressed(vs::InputAction::ToggleDebug)) {
                showGrid = !showGrid;
                std::printf("grilla %s\n", showGrid ? "ON" : "OFF");
                std::fflush(stdout);
            }

            previous = current;
            StepScene(current, input, fixedDelta);
        }
        const double physicsSeconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - physicsStart).count();

        // Si este frame no ejecuto ningun paso, `previous` y `current` siguen
        // iguales y alpha crece: la imagen sigue avanzando hacia el estado ya
        // simulado en lugar de congelarse hasta el siguiente paso.
        const float alpha = static_cast<float>(stepper.Alpha());
        camera.SetPosition(vs::Lerp(previous.cameraPosition, current.cameraPosition, alpha));

        // La ventana puede cambiar de tamano entre frames; el viewport y la
        // proyeccion tienen que seguirla o la imagen se estira.
        camera.SetViewport(window.FramebufferWidth(), window.FramebufferHeight());
        glViewport(0, 0, window.FramebufferWidth(), window.FramebufferHeight());
        glClearColor(vs::colors::kBackground.r, vs::colors::kBackground.g,
                     vs::colors::kBackground.b, vs::colors::kBackground.a);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer.Begin(camera);

        if (showGrid) {
            renderer.DrawGrid(camera);
            renderer.DrawAxes(camera);
        }

        // Tres formas con velocidades distintas: si giran suaves a 120 Hz, la
        // interpolacion esta funcionando; si dieran saltos, no.
        const float squareAngle = vs::AngleLerp(previous.squareAngle, current.squareAngle, alpha);
        const float barAngle = vs::AngleLerp(previous.barAngle, current.barAngle, alpha);
        const float circlePhase =
            previous.circlePhase + (current.circlePhase - previous.circlePhase) * alpha;
        const float circleRadius = 0.8F + 0.15F * std::sin(circlePhase);

        renderer.DrawRect({0.0F, 0.0F}, {0.7F, 0.7F}, squareAngle, vs::colors::kAccent);
        renderer.DrawCircle({3.0F, 0.0F}, circleRadius, vs::colors::kSecondary, 48);
        renderer.DrawRect({-3.0F, 1.5F}, {1.2F, 0.35F}, barAngle,
                          vs::Color{0.85F, 0.35F, 0.55F, 1.0F});

        renderer.End();

        window.SwapBuffers();

        stats.AddFrame(frameSeconds, physicsSeconds, stepCount);

        vs::FrameSnapshot snapshot;
        if (stats.TakeSnapshot(snapshot)) {
            char title[160];
            std::snprintf(title, sizeof(title),
                          "VehicleSim | %.1f fps | %.2f ms/frame | fisica %.3f ms | %.1f pasos",
                          snapshot.fps, snapshot.frameMs, snapshot.physicsMs,
                          snapshot.stepsPerFrame);
            window.SetTitle(title);

            std::printf("%.1f fps | frame %.2f ms | fisica %.3f ms | %.2f pasos/frame\n",
                        snapshot.fps, snapshot.frameMs, snapshot.physicsMs,
                        snapshot.stepsPerFrame);
            std::fflush(stdout);
        }
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
