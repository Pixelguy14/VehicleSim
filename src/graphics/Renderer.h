#pragma once

#include <vector>

#include <glad/gl.h>

#include "graphics/Color.h"
#include "graphics/Shader.h"
#include "math/Camera2D.h"
#include "math/Vec2.h"

namespace vs {

/// Vértice del lote: posicion en el mundo y color.
///
/// Seis floats, 24 bytes. Nada de UV ni de normal todavia: el proyecto dibuja
/// formas planas de color. Cuando lleguen texturas (minimapa, sprites) se
/// anadira el atributo y el resto del formato no cambia.
struct Vertex {
    float x = 0.0F;
    float y = 0.0F;
    float r = 1.0F;
    float g = 1.0F;
    float b = 1.0F;
    float a = 1.0F;
};

/// Renderer 2D por lotes.
///
/// Acumula todo lo que se le pide dibujar y lo envia a la GPU en una sola
/// llamada por tipo de primitiva al final del frame. Con 3000 lineas de grilla
/// eso es la diferencia entre 3000 draw calls y 1.
///
/// Las lineas van a un lote (GL_LINES) y las caras a otro (GL_TRIANGLES). Mezclar
/// ambos en un solo buffer obligaria a usar indices y a reordenar; con dos lotes
/// el codigo es mas corto y el coste es una segunda llamada.
///
/// Uso:
///
///     renderer.Begin(camera);   // fija proyeccion, vacia lotes
///     renderer.DrawGrid(camera);
///     renderer.DrawRect(...);
///     renderer.End();           // sube a la GPU y dibuja
///
/// Los colores se mezclan con alpha, no se reemplazan: asi el fondo oscuro
/// respira a traves de las lineas y una linea sobre otra se refuerza.
class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /// Prepara un frame con la proyeccion de `camera` y descarta lo acumulado
    /// del frame anterior (por si alguien olvido llamar a End).
    void Begin(const Camera2D& camera);

    /// Envia lo acumulado y lo dibuja.
    void End();

    /// Segmento. `thicknessPixels` de 1 o menos usa lineas nativas del driver;
    /// por encima se construye un quad, porque OpenGL 3.3 core no garantiza
    /// glLineWidth mayor que 1 y en la practica lo ignora.
    void DrawLine(Vec2 a, Vec2 b, Color color, float thicknessPixels = 1.0F);

    /// Rectangulo relleno, girado `angle` radianes alrededor de su centro.
    /// `halfExtent` es la mitad del ancho y de la altura, en metros.
    void DrawRect(Vec2 center, Vec2 halfExtent, float angle, Color color);

    /// Circulo relleno como abanico de triangulos.
    void DrawCircle(Vec2 center, float radius, Color color, int segments = 32);

    /// Grilla de referencia adaptativa: duplica el paso hasta que las lineas no
    /// queden mas juntas de unos 24 pixeles, de modo que al alejar la camara no
    /// se convierte en una masa solida.
    void DrawGrid(const Camera2D& camera, float baseStep = 1.0F, int majorEvery = 5);

    /// Ejes X (rojo) e Y (verde) del mundo, solo si caen dentro de la vista.
    void DrawAxes(const Camera2D& camera);

private:
    void PushLine(Vec2 point, Color color);
    void PushSegment(Vec2 a, Vec2 b, Color color);
    void PushTriangle(Vec2 a, Vec2 b, Vec2 c, Color color);
    void Upload(const std::vector<Vertex>& vertices, GLenum mode);
    [[nodiscard]] float ToWorldLength(float pixels) const;

    Shader shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLint uProjection_ = -1;

    std::vector<Vertex> lines_;
    std::vector<Vertex> triangles_;

    /// Pixeles por metro del frame en curso. Hace falta para que los grosores
    /// se pidan en pixeles y no en metros: un eje de 2 px debe verse igual con
    /// la camara cerca que lejos.
    float pixelsPerUnit_ = 1.0F;
};

}  // namespace vs
