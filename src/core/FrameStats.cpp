#include "core/FrameStats.h"

namespace vs {

void FrameStats::AddFrame(double wallSeconds, double physicsSeconds, int steps) {
    if (!(wallSeconds > 0.0)) {
        return;
    }

    windowSeconds_ += wallSeconds;
    physicsSeconds_ += physicsSeconds > 0.0 ? physicsSeconds : 0.0;
    steps_ += static_cast<double>(steps);
    ++frames_;
}

bool FrameStats::TakeSnapshot(FrameSnapshot& out) {
    // La ventana es de un segundo, con una microsegundo de tolerancia: sumar
    // 120 veces 1/120 en binario da 0.9999999999, no 1.0, y sin esta holgura la
    // primera medida llegaria un frame tarde por un error de redondeo. No es un
    // problema de precision del que preocuparse, pero si de uno que se nota al
    // escribir tests.
    constexpr double kWindowSeconds = 1.0;
    constexpr double kEpsilonSeconds = 1e-6;

    if (windowSeconds_ < kWindowSeconds - kEpsilonSeconds || frames_ == 0) {
        return false;
    }

    const double frames = static_cast<double>(frames_);
    out.fps = frames / windowSeconds_;
    out.frameMs = (windowSeconds_ / frames) * 1000.0;
    out.physicsMs = (physicsSeconds_ / frames) * 1000.0;
    out.stepsPerFrame = steps_ / frames;
    out.frames = frames_;

    Reset();
    return true;
}

void FrameStats::Reset() {
    windowSeconds_ = 0.0;
    physicsSeconds_ = 0.0;
    steps_ = 0.0;
    frames_ = 0;
}

}  // namespace vs
