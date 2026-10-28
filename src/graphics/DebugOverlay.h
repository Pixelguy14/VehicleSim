#pragma once

namespace vs {

class Window;

/// Superposicion de depuracion basada en Dear ImGui.
///
/// RAII: crea el contexto de ImGui y engancha los backends de GLFW y OpenGL en
/// el constructor, y los suelta en el destructor. Tiene que construirse despues
/// de la ventana (necesita su handle y su contexto) y destruirse antes que ella.
///
/// Ritmo de uso, una vez por frame:
///
///     overlay.BeginFrame();        // abre el frame de interfaz
///     ... widgets ...
///     glClear / dibujar el mundo ...
///     overlay.Render();            // dibuja la interfaz encima del mundo
class DebugOverlay {
public:
    explicit DebugOverlay(const Window& window);
    ~DebugOverlay();

    DebugOverlay(const DebugOverlay&) = delete;
    DebugOverlay& operator=(const DebugOverlay&) = delete;
    DebugOverlay(DebugOverlay&&) = delete;
    DebugOverlay& operator=(DebugOverlay&&) = delete;

    void BeginFrame();

    /// Genera la geometria de la interfaz y la dibuja. Debe ir despues del mundo:
    /// no limpia el color, se pinta encima.
    void Render();

    /// Si ImGui se esta quedando con el raton o con el teclado. Con esto se
    /// congela la entrada del juego mientras se arrastra un slider o se escribe
    /// en un campo, que es justo el gancho que expone InputManager desde 0.5b.
    [[nodiscard]] bool WantsMouse() const;
    [[nodiscard]] bool WantsKeyboard() const;
};

}  // namespace vs
