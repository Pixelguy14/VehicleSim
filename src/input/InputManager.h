#pragma once

#include "input/InputAction.h"
#include "input/InputState.h"

namespace vs {

class Window;

/// Traduce dispositivos fisicos (teclado y gamepad) a acciones logicas.
///
/// Ritmo de uso, desde el bucle principal:
///
///     input.Poll();                          // una vez por frame
///     for (cada paso fijo) {
///         input.BeginStep();                 // una vez por paso
///         ... logica que consulta acciones ...
///     }
///
/// Guarda una referencia a la ventana: el InputManager debe destruirse antes
/// que ella.
class InputManager {
public:
    explicit InputManager(const Window& window);

    /// Lee el hardware y actualiza los estados sostenidos y los flancos
    /// pendientes. Una vez por frame.
    void Poll();

    /// Publica los flancos pendientes al estado visible del paso. Una vez por
    /// paso fijo.
    void BeginStep();

    [[nodiscard]] bool IsDown(InputAction action) const;
    [[nodiscard]] bool WasPressed(InputAction action) const;
    [[nodiscard]] bool WasReleased(InputAction action) const;

    /// Direccion de giro: -1 izquierda, +1 derecha. Mezcla el teclado con el
    /// stick analogico del gamepad, que tiene prioridad cuando se sale de la
    /// zona muerta.
    [[nodiscard]] float SteerAxis() const;

    /// Acelerador y freno en 0..1. Soportan gatillos analogicos.
    [[nodiscard]] float ThrottleAxis() const;
    [[nodiscard]] float BrakeAxis() const;

    /// Mientras la interfaz tenga el teclado (un slider, un campo de texto), la
    /// entrada de juego se congela para no mover el vehiculo mientras se
    /// ajustan parametros. Al liberarla, la primera lectura puede generar un
    /// flanco si alguna tecla sigue pulsada: es correcto, la tecla esta pulsada.
    void SetUiCapture(bool captured);
    [[nodiscard]] bool IsUiCaptured() const { return uiCaptured_; }

    /// Suelta todas las acciones y descarta los flancos pendientes. Se usa al
    /// perder el foco de la ventana, cuando el sistema deja de enviar eventos y
    /// una tecla pulsada se quedaria pegada.
    void Reset();

    /// Zona muerta de los ejes analogicos, en 0..1. Por defecto 0.2.
    void SetDeadzone(float deadzone);

private:
    const Window& window_;
    InputState state_;

    bool uiCaptured_ = false;
    bool focused_ = true;
    bool padConnected_ = false;
    float padSteer_ = 0.0F;
    float padThrottle_ = 0.0F;
    float padBrake_ = 0.0F;
    float deadzone_ = 0.2F;
};

}  // namespace vs
