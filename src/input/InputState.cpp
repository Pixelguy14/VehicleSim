#include "input/InputState.h"

#include <cstddef>

namespace vs {

void InputState::UpdateRaw(InputAction action, bool down) {
    const std::size_t index = ToIndex(action);
    ActionState& state = state_[index];

    if (down && !state.down) {
        pendingPressed_[index] = true;
    }
    if (!down && state.down) {
        pendingReleased_[index] = true;
    }

    state.down = down;
}

void InputState::BeginStep() {
    for (std::size_t i = 0; i < kInputActionCount; ++i) {
        state_[i].pressed = pendingPressed_[i];
        state_[i].released = pendingReleased_[i];

        // Se consumen: si en este frame corren dos pasos, el segundo no vuelve
        // a ver la misma pulsacion.
        pendingPressed_[i] = false;
        pendingReleased_[i] = false;
    }
}

bool InputState::IsDown(InputAction action) const {
    return state_[ToIndex(action)].down;
}

bool InputState::WasPressed(InputAction action) const {
    return state_[ToIndex(action)].pressed;
}

bool InputState::WasReleased(InputAction action) const {
    return state_[ToIndex(action)].released;
}

const ActionState& InputState::Get(InputAction action) const {
    return state_[ToIndex(action)];
}

void InputState::Reset() {
    state_.fill(ActionState{});
    pendingPressed_.fill(false);
    pendingReleased_.fill(false);
}

}  // namespace vs
