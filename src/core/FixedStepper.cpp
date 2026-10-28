#include "core/FixedStepper.h"

#include <algorithm>
#include <cmath>

namespace vs {

FixedStepper::FixedStepper(double fixedDelta) : fixedDelta_(fixedDelta) {
    // Un paso de cero o negativo haria que Advance entrara en un bucle infinito:
    // mejor caer en el valor por defecto que colgarse por un error de tecleo.
    if (!(fixedDelta_ > 0.0) || !std::isfinite(fixedDelta_)) {
        fixedDelta_ = kDefaultFixedDelta;
    }
}

int FixedStepper::Advance(double frameSeconds) {
    droppedTime_ = false;

    // Escrito asi a proposito: con `frameSeconds != frameSeconds` (NaN) o un
    // valor negativo queremos salir sin tocar el acumulador.
    if (!(frameSeconds > 0.0)) {
        return 0;
    }

    if (frameSeconds > maxFrameSeconds_) {
        frameSeconds = maxFrameSeconds_;
        droppedTime_ = true;
    }

    accumulator_ += frameSeconds;

    int steps = 0;
    while (accumulator_ >= fixedDelta_ && steps < maxStepsPerFrame_) {
        accumulator_ -= fixedDelta_;
        ++steps;
    }

    // Se agoto el presupuesto de pasos y aun queda un paso entero pendiente: se
    // descarta. Dejarlo acumulado garantizaria que el siguiente frame vuelva a
    // pasarse, que es exactamente el spiral of death.
    if (accumulator_ >= fixedDelta_) {
        accumulator_ = std::fmod(accumulator_, fixedDelta_);
        droppedTime_ = true;
    }

    if (accumulator_ < 0.0) {
        accumulator_ = 0.0;
    }

    return steps;
}

double FixedStepper::Alpha() const {
    if (!(fixedDelta_ > 0.0)) {
        return 0.0;
    }

    const double alpha = accumulator_ / fixedDelta_;
    return std::clamp(alpha, 0.0, 1.0);
}

void FixedStepper::Reset() {
    accumulator_ = 0.0;
    droppedTime_ = false;
}

void FixedStepper::SetMaxFrameSeconds(double seconds) {
    if (seconds > 0.0 && std::isfinite(seconds)) {
        maxFrameSeconds_ = seconds;
    }
}

void FixedStepper::SetFixedDelta(double seconds) {
    // Misma validacion que el constructor: un paso cero colgaria Advance.
    if (!(seconds > 0.0) || !std::isfinite(seconds)) {
        return;
    }

    fixedDelta_ = seconds;

    // El acumulador se midio con el paso anterior. Si el paso nuevo es mas
    // pequeno, lo pendiente podria disparar varios pasos de golpe al cambiar
    // desde el panel; se conserva como mucho lo que cabe en un paso.
    if (accumulator_ >= fixedDelta_) {
        accumulator_ = std::fmod(accumulator_, fixedDelta_);
    }
}

void FixedStepper::SetMaxStepsPerFrame(int steps) {
    maxStepsPerFrame_ = std::max(steps, 1);
}

}  // namespace vs
