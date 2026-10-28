#include "graphics/DebugOverlay.h"

#include <stdexcept>

#include <glad/gl.h>  // siempre antes que GLFW: decide que cabecera de OpenGL se usa
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/Window.h"

namespace vs {
namespace {

// La misma version de GLSL que usa el renderer. Si aqui y alli no coinciden,
// ImGui dibuja igual (compila sus propios shaders) pero se pierde la ventaja de
// que un fallo de driver afecte a los dos por igual.
constexpr const char* kGlslVersion = "#version 330 core";

}  // namespace

DebugOverlay::DebugOverlay(const Window& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // Sin archivo .ini: un juego no debe escribir la posicion de sus ventanas de
    // depuracion junto al ejecutable, y en el instalador esa carpeta puede ser
    // de solo lectura. La posicion se fija en codigo.
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 6.0F;
    ImGui::GetStyle().FrameRounding = 4.0F;

    // install_callbacks = true: ImGui engancha sus callbacks de raton y teclado
    // y conserva los que ya existian (el de redimensionado de Window), asi que
    // no rompe nada del proyecto.
    if (!ImGui_ImplGlfw_InitForOpenGL(window.Handle(), true)) {
        ImGui::DestroyContext();
        throw std::runtime_error("No se pudo inicializar el backend GLFW de ImGui");
    }

    if (!ImGui_ImplOpenGL3_Init(kGlslVersion)) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        throw std::runtime_error("No se pudo inicializar el backend OpenGL de ImGui");
    }
}

DebugOverlay::~DebugOverlay() {
    // En orden inverso a la construccion: primero los backends, luego el
    // contexto que usan.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugOverlay::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugOverlay::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool DebugOverlay::WantsMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool DebugOverlay::WantsKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

}  // namespace vs
