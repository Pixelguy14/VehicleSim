#include <cstdio>
#include <cstring>

#include "core/Version.h"

namespace {

constexpr const char* kUsage =
    "Uso: vehicle_sim [opciones]\n"
    "\n"
    "Opciones:\n"
    "  --version   muestra la version y termina\n"
    "  --help      muestra esta ayuda y termina\n";

void PrintVersion() {
    std::printf("%s %s\n", vs::kProjectName, vs::kProjectVersion);
}

void PrintBanner() {
    std::printf("%s %s\n", vs::kProjectName, vs::kProjectVersion);
    std::printf("Simulacion 2D de trafico urbano\n");
    std::printf("Compilado con %s %s, configuracion %s\n", VS_COMPILER_ID, VS_COMPILER_VERSION,
                VS_BUILD_CONFIG);
    std::printf("\nTodavia sin ventana: GLFW llega en el Sprint 0.5.\n");
    std::printf("Prueba: vehicle_sim --version\n");
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
        if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
            std::printf("%s", kUsage);
            return 0;
        }
        std::fprintf(stderr, "Opcion desconocida: %s\n\n%s", arg, kUsage);
        return 2;
    }

    PrintBanner();
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    return Run(argc, argv);
}
