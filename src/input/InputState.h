#pragma once

#include <array>

#include "input/InputAction.h"

namespace vs {

/// Estado visible de una accion durante un paso de simulacion.
struct ActionState {
    bool down = false;      ///< Esta pulsada ahora mismo.
    bool pressed = false;   ///< Se pulso y este paso es el primero en enterarse.
    bool released = false;  ///< Se solto y este paso es el primero en enterarse.
};

/// Resuelve la diferencia de ritmo entre la lectura del hardware y el consumo
/// de la logica.
///
/// El problema concreto: el teclado se lee una vez por frame, a ritmo variable,
/// pero la simulacion avanza en pasos fijos. Si lees el hardware dentro del
/// paso fijo, una pulsacion se puede consumir dos veces (cuando en un frame
/// corren dos pasos) o perderse del todo (cuando en un frame no corre ninguno).
///
/// Esta clase acumula los flancos y los entrega exactamente una vez, al primer
/// paso que los consuma. Los estados sostenidos (`IsDown`) se refrescan en cada
/// lectura, asi que dentro de un mismo frame todos los pasos ven el mismo
/// mundo.
///
/// No conoce GLFW ni OpenGL: se puede probar sin abrir una ventana.
class InputState {
public:
    /// Registra el estado instantaneo de una accion. Se llama una vez por
    /// frame, con el estado leido del hardware.
    void UpdateRaw(InputAction action, bool down);

    /// Traslada los flancos acumulados al estado visible del paso. Se llama al
    /// principio de cada paso fijo, antes de ejecutar la logica del juego.
    void BeginStep();

    [[nodiscard]] bool IsDown(InputAction action) const;
    [[nodiscard]] bool WasPressed(InputAction action) const;
    [[nodiscard]] bool WasReleased(InputAction action) const;
    [[nodiscard]] const ActionState& Get(InputAction action) const;

    /// Vuelve al estado inicial y descarta los flancos pendientes.
    ///
    /// Se usa cuando la interfaz toma el control (un campo de texto) o cuando
    /// la ventana pierde el foco: al volver no queremos que se dispare una
    /// accion que ocurrio mientras no estabamos mirando.
    void Reset();

private:
    std::array<ActionState, kInputActionCount> state_{};
    std::array<bool, kInputActionCount> pendingPressed_{};
    std::array<bool, kInputActionCount> pendingReleased_{};
};

}  // namespace vs
