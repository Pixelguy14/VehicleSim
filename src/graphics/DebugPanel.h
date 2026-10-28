#pragma once

#include <array>
#include <cstddef>

#include "core/FrameStats.h"
#include "math/Vec2.h"

namespace vs {

class Camera2D;
class FixedStepper;

/// Todo lo que el panel puede leer o modificar.
///
/// Son punteros y no copias a proposito: los sliders tienen que cambiar el
/// estado real del bucle, no una copia que se descarta al terminar el frame.
/// Cualquier campo puede ser nulo y el panel simplemente no dibuja esa seccion,
/// lo que permite reutilizarlo cuando cambie la escena.
struct DebugPanelContext {
    const FrameSnapshot* snapshot = nullptr;
    FixedStepper* stepper = nullptr;
    Camera2D* camera = nullptr;
    Vec2* cameraPosition = nullptr;
    bool* showGrid = nullptr;
    bool* paused = nullptr;
};

/// Panel de depuracion en vivo.
///
/// Muestra el rendimiento medido y deja tocar los parametros del bucle sin
/// recompilar. Existe porque ajustar el paso fijo o el zoom a base de editar
/// constantes y relanzar el juego convierte una comprobacion de dos segundos en
/// un ciclo de dos minutos.
class DebugPanel {
public:
    /// Anade un frame a la grafica de tiempo. Se llama una vez por frame, con
    /// el tiempo real del frame en milisegundos.
    void PushFrameTime(float milliseconds);

    /// Construye los widgets. Debe llamarse entre BeginFrame y Render.
    void Draw(const DebugPanelContext& context);

private:
    // 120 muestras a 120 Hz son un segundo de historia: suficiente para ver un
    // atasco y volver a la normalidad sin que la grafica se vuelva ilegible.
    static constexpr std::size_t kHistorySize = 120;

    std::array<float, kHistorySize> history_{};
    std::size_t historyCount_ = 0;
};

}  // namespace vs
