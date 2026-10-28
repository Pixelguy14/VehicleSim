#pragma once

struct GLFWwindow;

namespace vs {

/// Configuracion inicial de la ventana.
struct WindowConfig {
    int width = 1280;
    int height = 720;
    const char* title = "VehicleSim";
    bool vsync = true;
};

/// Ventana GLFW con contexto OpenGL 3.3 core y glad ya cargado.
///
/// RAII: inicializa GLFW en el constructor y la termina en el destructor. Si
/// el constructor falla lanza std::runtime_error con el error de GLFW ya
/// traducido, de modo que quien la use no tenga que comprobar punteros nulos.
///
/// Asume una sola ventana por proceso, que es lo que necesita la aplicacion.
/// Si algun dia hicieran falta varias, habria que separar la vida de GLFW (que
/// es global) de la vida de cada ventana.
class Window {
public:
    explicit Window(const WindowConfig& config = {});
    ~Window();

    // Dos objetos sobre el mismo contexto OpenGL no tienen sentido: prohibido
    // copiar y tambien mover, porque GLFW identifica la ventana por puntero y
    // un movimiento dejaria el objeto original apuntando a algo destruido.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    void PollEvents();
    void SwapBuffers();

    [[nodiscard]] bool ShouldClose() const;
    void RequestClose();

    /// Titulo de la ventana. Se usa para mostrar FPS y coste de fisica en vivo
    /// sin depender de una interfaz grafica que todavia no existe.
    void SetTitle(const char* title);

    /// Si la ventana tiene el foco del sistema. Cuando lo pierde, GLFW deja de
    /// enviar eventos de teclado y las teclas pulsadas quedan congeladas.
    [[nodiscard]] bool IsFocused() const;

    /// Tamano del framebuffer en pixeles, no en unidades de pantalla. En
    /// pantallas HiDPI no coinciden, y es este el que espera glViewport.
    [[nodiscard]] int FramebufferWidth() const { return width_; }
    [[nodiscard]] int FramebufferHeight() const { return height_; }

    [[nodiscard]] GLFWwindow* Handle() const { return handle_; }

private:
    static void OnFramebufferResize(GLFWwindow* window, int width, int height);

    GLFWwindow* handle_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};

/// Version de GLFW en tiempo de ejecucion. Existe para que el resto del codigo
/// pueda informar de ella sin incluir cabeceras de GLFW.
[[nodiscard]] const char* GlfwVersion();

}  // namespace vs