#include "math/Camera2D.h"

#include <algorithm>

namespace vs {

void Camera2D::SetViewport(int widthPixels, int heightPixels) {
    // Una ventana minimizada reporta 0x0 y dividir por cero en la proyeccion
    // produce NaN que despues son muy dificiles de rastrear.
    width_ = std::max(widthPixels, 1);
    height_ = std::max(heightPixels, 1);
}

void Camera2D::SetZoom(float pixelsPerUnit) {
    zoom_ = std::clamp(pixelsPerUnit, 1.0F, 400.0F);
}

float Camera2D::HalfWidth() const {
    return static_cast<float>(width_) / (2.0F * zoom_);
}

float Camera2D::HalfHeight() const {
    return static_cast<float>(height_) / (2.0F * zoom_);
}

std::array<float, 16> Camera2D::ProjectionMatrix() const {
    const float halfWidth = HalfWidth();
    const float halfHeight = HalfHeight();

    // Ortografica centrada en la posicion de la camara. column-major:
    //   columna 0 = eje X, columna 1 = eje Y, columna 2 = eje Z, columna 3 = traslacion.
    std::array<float, 16> matrix{};
    matrix[0] = 1.0F / halfWidth;
    matrix[5] = 1.0F / halfHeight;
    matrix[10] = -1.0F;
    matrix[12] = -position_.x / halfWidth;
    matrix[13] = -position_.y / halfHeight;
    matrix[15] = 1.0F;
    return matrix;
}

Vec2 Camera2D::ScreenToWorld(Vec2 screenPixels) const {
    const float normalizedX = screenPixels.x / static_cast<float>(width_);
    const float normalizedY = screenPixels.y / static_cast<float>(height_);

    return {position_.x + (normalizedX - 0.5F) * 2.0F * HalfWidth(),
            position_.y + (0.5F - normalizedY) * 2.0F * HalfHeight()};
}

Vec2 Camera2D::WorldToScreen(Vec2 world) const {
    const float normalizedX = (world.x - position_.x) / (2.0F * HalfWidth()) + 0.5F;
    const float normalizedY = 0.5F - (world.y - position_.y) / (2.0F * HalfHeight());

    return {normalizedX * static_cast<float>(width_), normalizedY * static_cast<float>(height_)};
}

}  // namespace vs
