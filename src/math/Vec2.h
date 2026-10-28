#pragma once

#include <cmath>

namespace vs {

/// Vector 2D de proposito general.
///
/// Prefijo `vs` para no colisionar con `b2Vec2` de Box2D: los dos conviven en
/// el proyecto y tenerlos separados evita confusiones sobre cual se esta usando
/// en cada punto. La conversion entre ambos se hace en la capa de fisica.
struct Vec2 {
    float x = 0.0F;
    float y = 0.0F;
};

constexpr Vec2 operator+(Vec2 a, Vec2 b) {
    return {a.x + b.x, a.y + b.y};
}

constexpr Vec2 operator-(Vec2 a, Vec2 b) {
    return {a.x - b.x, a.y - b.y};
}

constexpr Vec2 operator-(Vec2 v) {
    return {-v.x, -v.y};
}

constexpr Vec2 operator*(Vec2 v, float scalar) {
    return {v.x * scalar, v.y * scalar};
}

constexpr Vec2 operator*(float scalar, Vec2 v) {
    return v * scalar;
}

constexpr Vec2 operator/(Vec2 v, float scalar) {
    return {v.x / scalar, v.y / scalar};
}

constexpr Vec2& operator+=(Vec2& a, Vec2 b) {
    a = a + b;
    return a;
}

constexpr Vec2& operator-=(Vec2& a, Vec2 b) {
    a = a - b;
    return a;
}

constexpr Vec2& operator*=(Vec2& v, float scalar) {
    v = v * scalar;
    return v;
}

constexpr bool operator==(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

constexpr float Dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

/// Producto cruz en 2D: positivo cuando `b` queda a la izquierda de `a`.
constexpr float Cross(Vec2 a, Vec2 b) {
    return a.x * b.y - a.y * b.x;
}

constexpr float LengthSquared(Vec2 v) {
    return Dot(v, v);
}

inline float Length(Vec2 v) {
    return std::sqrt(LengthSquared(v));
}

inline Vec2 Normalized(Vec2 v) {
    const float length = Length(v);
    return length > 0.0F ? v / length : Vec2{};
}

/// Gira `v` `radians` en sentido antihorario.
inline Vec2 Rotated(Vec2 v, float radians) {
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    return {v.x * cosine - v.y * sine, v.x * sine + v.y * cosine};
}

constexpr Vec2 Lerp(Vec2 a, Vec2 b, float t) {
    return a + (b - a) * t;
}

inline float Angle(Vec2 v) {
    return std::atan2(v.y, v.x);
}

}  // namespace vs
