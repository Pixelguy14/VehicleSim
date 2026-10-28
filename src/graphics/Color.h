#pragma once

namespace vs {

/// Color RGBA en coma flotante, 0..1. Es el formato que espera OpenGL y evita
/// conversiones desde bytes en cada frame.
struct Color {
    float r = 1.0F;
    float g = 1.0F;
    float b = 1.0F;
    float a = 1.0F;
};

namespace colors {

inline constexpr Color kWhite{1.0F, 1.0F, 1.0F, 1.0F};
inline constexpr Color kBackground{0.09F, 0.10F, 0.12F, 1.0F};

inline constexpr Color kGridMinor{0.20F, 0.22F, 0.26F, 1.0F};
inline constexpr Color kGridMajor{0.31F, 0.35F, 0.41F, 1.0F};
inline constexpr Color kAxisX{0.78F, 0.31F, 0.31F, 1.0F};
inline constexpr Color kAxisY{0.36F, 0.72F, 0.42F, 1.0F};

inline constexpr Color kAccent{0.95F, 0.70F, 0.25F, 1.0F};
inline constexpr Color kSecondary{0.36F, 0.62F, 0.95F, 1.0F};

}  // namespace colors

}  // namespace vs
