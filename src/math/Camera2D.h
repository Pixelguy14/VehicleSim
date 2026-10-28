#pragma once

#include <array>

#include "math/Vec2.h"

namespace vs {

/// Camara ortografica 2D.
///
/// Convencion del proyecto: el mundo tiene Y hacia arriba y sus unidades son
/// metros de Box2D. La pantalla tiene Y hacia abajo, y esa conversion solo
/// ocurre aqui, en ScreenToWorld y WorldToScreen. En ningun otro sitio se
/// mezclan las dos.
class Camera2D {
public:
    void SetViewport(int widthPixels, int heightPixels);
    void SetPosition(Vec2 position) { position_ = position; }

    /// Pixeles por unidad de mundo. Un valor mayor acerca la camara.
    void SetZoom(float pixelsPerUnit);
    void Move(Vec2 delta) { position_ += delta; }

    [[nodiscard]] Vec2 Position() const { return position_; }
    [[nodiscard]] float Zoom() const { return zoom_; }
    [[nodiscard]] int ViewportWidth() const { return width_; }
    [[nodiscard]] int ViewportHeight() const { return height_; }

    /// Media anchura y media altura del area visible, en unidades de mundo.
    [[nodiscard]] float HalfWidth() const;
    [[nodiscard]] float HalfHeight() const;

    /// Matriz de proyeccion en orden column-major, lista para
    /// glUniformMatrix4fv con transpose = GL_FALSE.
    [[nodiscard]] std::array<float, 16> ProjectionMatrix() const;

    /// `screenPixels` con origen en la esquina superior izquierda.
    [[nodiscard]] Vec2 ScreenToWorld(Vec2 screenPixels) const;
    [[nodiscard]] Vec2 WorldToScreen(Vec2 world) const;

private:
    Vec2 position_{0.0F, 0.0F};
    float zoom_ = 40.0F;
    int width_ = 1;
    int height_ = 1;
};

}  // namespace vs
