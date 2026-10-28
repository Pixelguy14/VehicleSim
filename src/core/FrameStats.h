#pragma once

namespace vs {

/// Medidas de un segundo de ejecucion.
struct FrameSnapshot {
    double fps = 0.0;
    double frameMs = 0.0;
    double physicsMs = 0.0;
    double stepsPerFrame = 0.0;
    int frames = 0;
};

/// Acumula el coste de cada frame y lo publica una vez por segundo.
///
/// Se publica por ventana de tiempo y no por frame porque un solo frame no dice
/// nada: el ruido del sistema operativo hace que dos frames seguidos difieran en
/// un factor de tres. Un segundo de media es la cifra minima que se puede leer
/// sin enganarse.
class FrameStats {
public:
    void AddFrame(double wallSeconds, double physicsSeconds, int steps);

    /// Devuelve true como mucho una vez por segundo y rellena `out`. Al devolver
    /// true, la ventana se reinicia.
    [[nodiscard]] bool TakeSnapshot(FrameSnapshot& out);

    void Reset();

private:
    double windowSeconds_ = 0.0;
    double physicsSeconds_ = 0.0;
    double steps_ = 0.0;
    int frames_ = 0;
};

}  // namespace vs
