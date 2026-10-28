#pragma once

#include <cstddef>
#include <cstdint>

namespace vs {

/// Acciones logicas del juego.
///
/// El codigo de juego nunca pregunta por una tecla fisica: pregunta por una
/// accion. Eso permite reasignar controles, anadir gamepad o tener perfiles
/// distintos sin tocar la logica de conduccion ni la de camara.
enum class InputAction : std::uint8_t {
    Throttle = 0,
    Brake,
    SteerLeft,
    SteerRight,
    Handbrake,
    ToggleDebug,
    TogglePause,
    Reset,
    Quit,

    Count
};

inline constexpr std::size_t kInputActionCount = static_cast<std::size_t>(InputAction::Count);

/// Indice de array para una accion. Centralizado aqui para que nadie tenga que
/// repetir el static_cast por el codigo.
[[nodiscard]] constexpr std::size_t ToIndex(InputAction action) {
    return static_cast<std::size_t>(action);
}

/// Nombre legible, para logs y mensajes de depuracion.
[[nodiscard]] constexpr const char* ToString(InputAction action) {
    switch (action) {
        case InputAction::Throttle:
            return "Throttle";
        case InputAction::Brake:
            return "Brake";
        case InputAction::SteerLeft:
            return "SteerLeft";
        case InputAction::SteerRight:
            return "SteerRight";
        case InputAction::Handbrake:
            return "Handbrake";
        case InputAction::ToggleDebug:
            return "ToggleDebug";
        case InputAction::TogglePause:
            return "TogglePause";
        case InputAction::Reset:
            return "Reset";
        case InputAction::Quit:
            return "Quit";
        case InputAction::Count:
            break;
    }
    return "Desconocida";
}

}  // namespace vs
