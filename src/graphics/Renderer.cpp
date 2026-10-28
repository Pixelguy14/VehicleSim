#include "graphics/Renderer.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "math/Vec2.h"

namespace vs {
namespace {

constexpr float kTwoPi = 6.28318530718F;

// Adaptacion de la grilla: se duplica el paso hasta que las lineas queden al
// menos a esta distancia en pantalla. Por debajo, la grilla se vuelve una
// textura gris en lugar de una referencia util.
constexpr float kMinGridSpacingPixels = 24.0F;

// Tope de seguridad. Si alguien pide un paso absurdamente pequeno, es mejor no
// dibujar la grilla que colgar el frame generando millones de vertices.
constexpr int kMaxGridLines = 4000;

constexpr const char* kVertexSource = R"(#version 330 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec4 aColor;

uniform mat4 uProjection;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
}
)";

constexpr const char* kFragmentSource = R"(#version 330 core
in vec4 vColor;

out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";

}  // namespace

Renderer::Renderer() : shader_(kVertexSource, kFragmentSource) {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Atributo 0: posicion (2 floats, offset 0).
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(0));

    // Atributo 1: color (4 floats, tras la posicion).
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<const void*>(static_cast<std::uintptr_t>(
                              offsetof(Vertex, r))));

    glBindVertexArray(0);

    uProjection_ = shader_.Uniform("uProjection");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer::~Renderer() {
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
}

void Renderer::Begin(const Camera2D& camera) {
    lines_.clear();
    triangles_.clear();

    pixelsPerUnit_ = camera.Zoom();

    shader_.Bind();
    const std::array<float, 16> projection = camera.ProjectionMatrix();
    glUniformMatrix4fv(uProjection_, 1, GL_FALSE, projection.data());
}

void Renderer::End() {
    if (!lines_.empty()) {
        Upload(lines_, GL_LINES);
        lines_.clear();
    }
    if (!triangles_.empty()) {
        Upload(triangles_, GL_TRIANGLES);
        triangles_.clear();
    }
    glBindVertexArray(0);
}

float Renderer::ToWorldLength(float pixels) const {
    return pixels / pixelsPerUnit_;
}

void Renderer::PushLine(Vec2 point, Color color) {
    lines_.push_back(Vertex{point.x, point.y, color.r, color.g, color.b, color.a});
}

void Renderer::PushSegment(Vec2 a, Vec2 b, Color color) {
    PushLine(a, color);
    PushLine(b, color);
}

void Renderer::PushTriangle(Vec2 a, Vec2 b, Vec2 c, Color color) {
    triangles_.push_back(Vertex{a.x, a.y, color.r, color.g, color.b, color.a});
    triangles_.push_back(Vertex{b.x, b.y, color.r, color.g, color.b, color.a});
    triangles_.push_back(Vertex{c.x, c.y, color.r, color.g, color.b, color.a});
}

void Renderer::Upload(const std::vector<Vertex>& vertices, GLenum mode) {
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STREAM_DRAW);
    glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
}

void Renderer::DrawLine(Vec2 a, Vec2 b, Color color, float thicknessPixels) {
    if (thicknessPixels <= 1.0F) {
        PushSegment(a, b, color);
        return;
    }

    const Vec2 delta = b - a;
    const float length = Length(delta);
    if (length <= 0.0F) {
        return;
    }

    const Vec2 direction = delta / length;
    const Vec2 normal{-direction.y, direction.x};
    const Vec2 offset = normal * (ToWorldLength(thicknessPixels) * 0.5F);

    const Vec2 p0 = a + offset;
    const Vec2 p1 = b + offset;
    const Vec2 p2 = b - offset;
    const Vec2 p3 = a - offset;

    PushTriangle(p0, p1, p2, color);
    PushTriangle(p0, p2, p3, color);
}

void Renderer::DrawRect(Vec2 center, Vec2 halfExtent, float angle, Color color) {
    const Vec2 local[4] = {
        {-halfExtent.x, -halfExtent.y},
        {halfExtent.x, -halfExtent.y},
        {halfExtent.x, halfExtent.y},
        {-halfExtent.x, halfExtent.y},
    };

    Vec2 corner[4];
    for (int i = 0; i < 4; ++i) {
        corner[i] = center + Rotated(local[i], angle);
    }

    PushTriangle(corner[0], corner[1], corner[2], color);
    PushTriangle(corner[0], corner[2], corner[3], color);
}

void Renderer::DrawCircle(Vec2 center, float radius, Color color, int segments) {
    if (segments < 3) {
        segments = 3;
    }

    const float step = kTwoPi / static_cast<float>(segments);
    for (int i = 0; i < segments; ++i) {
        const float a0 = step * static_cast<float>(i);
        const float a1 = step * static_cast<float>(i + 1);
        PushTriangle(center, center + Vec2{std::cos(a0), std::sin(a0)} * radius,
                     center + Vec2{std::cos(a1), std::sin(a1)} * radius, color);
    }
}

void Renderer::DrawGrid(const Camera2D& camera, float baseStep, int majorEvery) {
    if (baseStep <= 0.0F) {
        return;
    }
    if (majorEvery < 1) {
        majorEvery = 1;
    }

    float step = baseStep;
    for (int guard = 0; guard < 24 && step * camera.Zoom() < kMinGridSpacingPixels; ++guard) {
        step *= 2.0F;
    }

    const Vec2 center = camera.Position();
    const float minX = center.x - camera.HalfWidth();
    const float maxX = center.x + camera.HalfWidth();
    const float minY = center.y - camera.HalfHeight();
    const float maxY = center.y + camera.HalfHeight();

    const int firstColumn = static_cast<int>(std::floor(minX / step));
    const int lastColumn = static_cast<int>(std::ceil(maxX / step));
    const int firstRow = static_cast<int>(std::floor(minY / step));
    const int lastRow = static_cast<int>(std::ceil(maxY / step));

    if (lastColumn - firstColumn > kMaxGridLines || lastRow - firstRow > kMaxGridLines) {
        return;
    }

    for (int column = firstColumn; column <= lastColumn; ++column) {
        // El eje lo dibuja DrawAxes con su propio color y grosor.
        if (column == 0) {
            continue;
        }
        const float x = static_cast<float>(column) * step;
        const Color color = (column % majorEvery == 0) ? colors::kGridMajor : colors::kGridMinor;
        PushSegment({x, minY}, {x, maxY}, color);
    }

    for (int row = firstRow; row <= lastRow; ++row) {
        if (row == 0) {
            continue;
        }
        const float y = static_cast<float>(row) * step;
        const Color color = (row % majorEvery == 0) ? colors::kGridMajor : colors::kGridMinor;
        PushSegment({minX, y}, {maxX, y}, color);
    }
}

void Renderer::DrawAxes(const Camera2D& camera) {
    const Vec2 center = camera.Position();
    const float minX = center.x - camera.HalfWidth();
    const float maxX = center.x + camera.HalfWidth();
    const float minY = center.y - camera.HalfHeight();
    const float maxY = center.y + camera.HalfHeight();

    if (minY <= 0.0F && maxY >= 0.0F) {
        DrawLine({minX, 0.0F}, {maxX, 0.0F}, colors::kAxisX, 2.0F);
    }
    if (minX <= 0.0F && maxX >= 0.0F) {
        DrawLine({0.0F, minY}, {0.0F, maxY}, colors::kAxisY, 2.0F);
    }
}

}  // namespace vs
