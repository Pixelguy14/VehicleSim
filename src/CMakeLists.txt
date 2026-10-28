# La version vive en un solo lugar: el project(... VERSION ...) del
# CMakeLists raiz. Aqui solo se materializa en una cabecera que el codigo
# puede incluir, para que nunca haya dos numeros de version que se contradigan.
set(VS_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/core/Version.h.in"
    "${VS_GENERATED_DIR}/core/Version.h"
    @ONLY
)

# --- Nucleo: logica sin OpenGL, GLFW ni ventana ------------------------------
#
# Separado en su propia biblioteca por una razon concreta: los tests enlazan
# contra esto y corren sin display, sin driver de GPU y sin ventana. Un test que
# necesita abrir una ventana no se puede ejecutar en un CI headless, y uno que
# necesita una GPU no se puede ejecutar en el portatil de otro.
#
# La regla para decidir donde va un archivo nuevo: si incluye <glad/gl.h> o
# <GLFW/glfw3.h>, va al ejecutable; si no, va aqui.
add_library(vs_core STATIC
    core/FixedStepper.cpp
    core/FrameStats.cpp
    input/InputState.cpp
    math/Camera2D.cpp
)

target_include_directories(vs_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

vs_apply_warnings(vs_core)
vs_apply_project_options(vs_core)

# --- Ejecutable --------------------------------------------------------------
add_executable(vehicle_sim
    main.cpp
    core/Window.cpp
    graphics/DebugOverlay.cpp
    graphics/DebugPanel.cpp
    graphics/Renderer.cpp
    graphics/Shader.cpp
    input/InputManager.cpp
)

target_include_directories(vehicle_sim
    PRIVATE
        "${VS_GENERATED_DIR}"
)

# Datos que solo el compilador conoce en tiempo de build. Se pasan como
# definiciones en lugar de escribir un archivo, para que funcionen igual en
# builds de una sola configuracion y en los de configuracion multiple (MSVC).
target_compile_definitions(vehicle_sim
    PRIVATE
        VS_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
        VS_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}"
        VS_BUILD_CONFIG="$<CONFIG>"
)

target_link_libraries(vehicle_sim
    PRIVATE
        vs_core
        vs_imgui
        glad::glad
        glfw
        box2d::box2d
)

vs_apply_warnings(vehicle_sim)
vs_apply_project_options(vehicle_sim)

install(TARGETS vehicle_sim
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Prueba de humo como test de ctest, para que corra tambien en los tres
# runners de integracion continua. --selftest no abre ventana ni inicializa
# GLFW (solo pide su version y simula un circulo cayendo con Box2D), asi que
# funciona en un contenedor sin servidor grafico.
if(BUILD_TESTING)
    add_test(NAME vs_selftest COMMAND vehicle_sim --selftest)
    set_tests_properties(vs_selftest PROPERTIES TIMEOUT 60)
endif()

# Atajo de desarrollo: compila y lanza el juego en un solo paso.
#
#     cmake --build --preset dev-debug --target run
#
# La ruta del ejecutable cambia segun el generador y la configuracion
# (build/dev-debug/src/vehicle_sim, build/dev-release/src/vehicle_sim, o
# build/ci/src/Debug/vehicle_sim.exe en Visual Studio), asi que se usa
# $<TARGET_FILE:...> en lugar de escribirla a mano. USES_TERMINAL hace que ninja
# deje pasar la salida del juego a la terminal en vez de tragarsela.
add_custom_target(run
    COMMAND $<TARGET_FILE:vehicle_sim>
    DEPENDS vehicle_sim
    USES_TERMINAL
)
