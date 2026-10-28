#define GLFW_INCLUDE_NONE

#include "core/Window.h"

#include <cstdio>
#include <stdexcept>
#include <string>

#include <glad/gl.h>  // siempre antes que GLFW: decide que cabecera de OpenGL se usa
#include <GLFW/glfw3.h>

namespace vs {
namespace {

void GlfwErrorCallback(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description != nullptr ? description : "-");
}

/// Recupera el ultimo error de GLFW como texto legible. Hay que llamarlo antes
/// de cualquier otra funcion de GLFW, porque cada llamada puede sobrescribirlo.
std::string DescribeGlfwError(const char* what) {
    const char* description = nullptr;
    const int code = glfwGetError(&description);

    std::string message = what;
    message += " (GLFW ";
    message += std::to_string(code);
    message += "): ";
    message += description != nullptr ? description : "sin descripcion";
    return message;
}

void PrintContextInfo() {
    const auto* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const auto* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const auto* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const auto* glsl = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    std::printf("OpenGL   %s\n", version != nullptr ? version : "desconocido");
    std::printf("Renderer %s\n", renderer != nullptr ? renderer : "desconocido");
    std::printf("Vendor   %s\n", vendor != nullptr ? vendor : "desconocido");
    std::printf("GLSL     %s\n", glsl != nullptr ? glsl : "desconocido");
}

}  // namespace

Window::Window(const WindowConfig& config) {
    // Antes de glfwInit: si se instala despues, los errores de inicializacion
    // se pierden y solo se ve un false sin explicacion.
    glfwSetErrorCallback(&GlfwErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error(DescribeGlfwError("No se pudo inicializar GLFW"));
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // macOS solo da contextos 3.3 core si son forward-compatible.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    // Se crea oculta y se muestra al final: sin esto aparece un parpadeo negro
    // entre la creacion de la ventana y el primer frame.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    handle_ = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
    if (handle_ == nullptr) {
        const std::string message = DescribeGlfwError("No se pudo crear la ventana OpenGL 3.3");
        glfwTerminate();
        throw std::runtime_error(message);
    }

    glfwMakeContextCurrent(handle_);

    const int loadedVersion = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
    if (loadedVersion == 0) {
        glfwDestroyWindow(handle_);
        glfwTerminate();
        throw std::runtime_error("glad no pudo cargar las funciones de OpenGL");
    }

    glfwSwapInterval(config.vsync ? 1 : 0);

    glfwSetWindowUserPointer(handle_, this);
    glfwSetFramebufferSizeCallback(handle_, &Window::OnFramebufferResize);

    glfwGetFramebufferSize(handle_, &width_, &height_);
    glViewport(0, 0, width_, height_);

    PrintContextInfo();
    glfwShowWindow(handle_);
}

Window::~Window() {
    if (handle_ != nullptr) {
        glfwDestroyWindow(handle_);
    }
    glfwTerminate();
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(handle_);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(handle_) == GLFW_TRUE;
}

void Window::RequestClose() {
    glfwSetWindowShouldClose(handle_, GLFW_TRUE);
}

bool Window::IsFocused() const {
    return glfwGetWindowAttrib(handle_, GLFW_FOCUSED) == GLFW_TRUE;
}

const char* GlfwVersion() {
    return glfwGetVersionString();
}

void Window::OnFramebufferResize(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self == nullptr) {
        return;
    }

    self->width_ = width;
    self->height_ = height;

    // Con la ventana minimizada el framebuffer mide 0x0. glViewport lo admite,
    // pero conviene no dibujar en ese estado (se comprueba en el renderer).
    glViewport(0, 0, width, height);
}

}  // namespace vs
