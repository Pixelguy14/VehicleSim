#define GLFW_INCLUDE_NONE

#include "input/InputManager.h"

#include <algorithm>
#include <array>
#include <cmath>

#include <GLFW/glfw3.h>

#include "core/Window.h"

namespace vs {
namespace {

struct KeyBinding {
    InputAction action;
    int key;
};

// Una accion puede tener varias teclas: WASD y flechas conviven.
constexpr std::array<KeyBinding, 13> kKeyBindings{{
    {InputAction::Throttle, GLFW_KEY_W},
    {InputAction::Throttle, GLFW_KEY_UP},
    {InputAction::Brake, GLFW_KEY_S},
    {InputAction::Brake, GLFW_KEY_DOWN},
    {InputAction::SteerLeft, GLFW_KEY_A},
    {InputAction::SteerLeft, GLFW_KEY_LEFT},
    {InputAction::SteerRight, GLFW_KEY_D},
    {InputAction::SteerRight, GLFW_KEY_RIGHT},
    {InputAction::Handbrake, GLFW_KEY_SPACE},
    {InputAction::ToggleDebug, GLFW_KEY_F1},
    {InputAction::TogglePause, GLFW_KEY_F2},
    {InputAction::Reset, GLFW_KEY_R},
    {InputAction::Quit, GLFW_KEY_ESCAPE},
}};

struct ButtonBinding {
    InputAction action;
    int button;
};

constexpr std::array<ButtonBinding, 5> kButtonBindings{{
    {InputAction::Handbrake, GLFW_GAMEPAD_BUTTON_A},
    {InputAction::ToggleDebug, GLFW_GAMEPAD_BUTTON_X},
    {InputAction::TogglePause, GLFW_GAMEPAD_BUTTON_Y},
    {InputAction::Reset, GLFW_GAMEPAD_BUTTON_B},
    {InputAction::Quit, GLFW_GAMEPAD_BUTTON_START},
}};

/// Aplica zona muerta y reescala el resto, para que el salto del umbral no se
/// sienta como un escalon al conducir.
float ApplyDeadzone(float value, float deadzone) {
    const float magnitude = std::abs(value);
    if (magnitude <= deadzone) {
        return 0.0F;
    }
    const float scaled = (magnitude - deadzone) / (1.0F - deadzone);
    return value < 0.0F ? -scaled : scaled;
}

bool ReadAction(GLFWwindow* window, InputAction action, const GLFWgamepadstate& pad,
                bool padReady) {
    for (const KeyBinding& binding : kKeyBindings) {
        if (binding.action != action) {
            continue;
        }
        if (glfwGetKey(window, binding.key) == GLFW_PRESS) {
            return true;
        }
    }

    if (!padReady) {
        return false;
    }

    for (const ButtonBinding& binding : kButtonBindings) {
        if (binding.action != action) {
            continue;
        }
        if (pad.buttons[binding.button] == GLFW_PRESS) {
            return true;
        }
    }

    return false;
}

}  // namespace

InputManager::InputManager(const Window& window) : window_(window) {}

void InputManager::Poll() {
    // Congelado mientras la interfaz tiene el teclado: no se lee hardware y no
    // se generan flancos nuevos.
    if (uiCaptured_) {
        return;
    }

    // Al perder el foco, el sistema deja de enviar eventos y una tecla que
    // estaba pulsada nunca llegaria a soltarse: el vehiculo se quedaria
    // acelerando solo. Se suelta todo una vez y no se vuelve a leer hasta
    // recuperar el foco.
    if (!window_.IsFocused()) {
        if (focused_) {
            state_.Reset();
            padSteer_ = 0.0F;
            padThrottle_ = 0.0F;
            padBrake_ = 0.0F;
            padConnected_ = false;
            focused_ = false;
        }
        return;
    }
    focused_ = true;

    GLFWgamepadstate pad{};
    padConnected_ = glfwGetGamepadState(GLFW_JOYSTICK_1, &pad) == GLFW_TRUE;

    for (std::size_t i = 0; i < kInputActionCount; ++i) {
        const auto action = static_cast<InputAction>(i);
        state_.UpdateRaw(action, ReadAction(window_.Handle(), action, pad, padConnected_));
    }

    padSteer_ =
        padConnected_ ? ApplyDeadzone(pad.axes[GLFW_GAMEPAD_AXIS_LEFT_X], deadzone_) : 0.0F;
    padThrottle_ = padConnected_ ? pad.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] : 0.0F;
    padBrake_ = padConnected_ ? pad.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] : 0.0F;
}

void InputManager::BeginStep() {
    state_.BeginStep();
}

bool InputManager::IsDown(InputAction action) const {
    return state_.IsDown(action);
}

bool InputManager::WasPressed(InputAction action) const {
    return state_.WasPressed(action);
}

bool InputManager::WasReleased(InputAction action) const {
    return state_.WasReleased(action);
}

float InputManager::SteerAxis() const {
    if (std::abs(padSteer_) > 0.0F) {
        return padSteer_;
    }

    float axis = 0.0F;
    if (state_.IsDown(InputAction::SteerLeft)) {
        axis -= 1.0F;
    }
    if (state_.IsDown(InputAction::SteerRight)) {
        axis += 1.0F;
    }
    return axis;
}

float InputManager::ThrottleAxis() const {
    const float keyboard = state_.IsDown(InputAction::Throttle) ? 1.0F : 0.0F;
    return std::max(padThrottle_, keyboard);
}

float InputManager::BrakeAxis() const {
    const float keyboard = state_.IsDown(InputAction::Brake) ? 1.0F : 0.0F;
    return std::max(padBrake_, keyboard);
}

void InputManager::SetUiCapture(bool captured) {
    if (captured == uiCaptured_) {
        return;
    }

    uiCaptured_ = captured;
    if (captured) {
        state_.Reset();
    }
}

void InputManager::Reset() {
    state_.Reset();
    padSteer_ = 0.0F;
    padThrottle_ = 0.0F;
    padBrake_ = 0.0F;
}

void InputManager::SetDeadzone(float deadzone) {
    deadzone_ = std::clamp(deadzone, 0.0F, 0.9F);
}

}  // namespace vs
