#pragma once

#include <cmath>

namespace vs {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kTwoPi = 6.28318530717958647692F;

/// Lleva un angulo a (-pi, pi].
///
/// Los angulos que devuelve Box2D ya vienen acotados, pero los que acumula el
/// juego (un volante girando, una rueda) crecen sin limite hasta que la
/// precision de float empieza a notarse. Acotarlos cada paso mantiene los
/// numeros pequenos y hace que dos estados equivalentes se comparen igual.
inline float WrapAngle(float radians) {
    if (!std::isfinite(radians)) {
        return 0.0F;
    }

    float wrapped = std::fmod(radians + kPi, kTwoPi);
    if (wrapped < 0.0F) {
        wrapped += kTwoPi;
    }
    return wrapped - kPi;
}

/// Distancia angular mas corta de `from` a `to`, en (-pi, pi].
inline float ShortestAngleDelta(float from, float to) {
    return WrapAngle(to - from);
}

/// Interpola entre dos angulos por el camino corto.
///
/// La interpolacion normal (a + (b - a) * t) es incorrecta para angulos: entre
/// 350 grados y 10 grados recorreria 340 grados hacia atras en lugar de 20 hacia
/// delante, y en pantalla se ve como un giro completo de golpe. Este es el error
/// clasico del render interpolado y ocurre justo cuando un coche cruza el eje.
inline float AngleLerp(float from, float to, float t) {
    return from + ShortestAngleDelta(from, to) * t;
}

}  // namespace vs
