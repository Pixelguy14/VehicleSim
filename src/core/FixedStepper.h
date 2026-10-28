#pragma once

namespace vs {

/// Convierte tiempo de pared irregular en pasos de simulacion de duracion fija.
///
/// El problema: el render corre a lo que de el monitor (60, 120, 144 Hz) y la
/// fisica no puede depender de eso, o el mismo coche frena distinto en cada
/// maquina y una repeticion deja de reproducirse. La solucion clasica es
/// acumular el tiempo real y consumirlo en porciones iguales.
///
/// El peligro del acumulador es el *spiral of death*: si un frame tarda 200 ms,
/// tocan 12 pasos, que tardan mas de 200 ms, que hacen que el siguiente frame
/// tarde aun mas. Por eso hay dos topes: el tiempo de un frame se acota y el
/// numero de pasos por frame tambien. Cuando se toca el tope el tiempo sobrante
/// se descarta: es preferible que la simulacion vaya un poco por detras a que
/// el juego se congele.
///
/// No conoce OpenGL ni GLFW: se puede probar con numeros, sin ventana.
///
/// Nota sobre aritmetica binaria: sumar N veces 1/N no da exactamente 1, asi que
/// a 144 Hz el paso 60 puede caer en el frame 145 y no en el 144. El tiempo no se
/// pierde (queda en el acumulador y se consume en el frame siguiente): solo se
/// reparte distinto entre frames. Por eso el numero de pasos puede diferir en uno
/// entre dos refrescos distintos, pero el tiempo simulado nunca diverge.
class FixedStepper {
public:
    static constexpr double kDefaultFixedDelta = 1.0 / 60.0;

    explicit FixedStepper(double fixedDelta = kDefaultFixedDelta);

    /// Consume el tiempo transcurrido y devuelve cuantos pasos fijos hay que
    /// ejecutar. Tras la llamada el tiempo sobrante queda en el acumulador.
    int Advance(double frameSeconds);

    /// Fraccion del siguiente paso ya acumulada, en [0, 1). Es el `alpha` que
    /// usa el render para interpolar entre el estado anterior y el actual.
    [[nodiscard]] double Alpha() const;

    [[nodiscard]] double FixedDelta() const { return fixedDelta_; }

    /// Cambia la duracion del paso. Se usa desde el panel de depuracion para
    /// comparar el coste de la fisica a 30, 60 o 120 Hz sin recompilar.
    void SetFixedDelta(double seconds);

    /// Tiempo acumulado sin consumir, en segundos.
    [[nodiscard]] double AccumulatorSeconds() const { return accumulator_; }

    /// Descarta el tiempo acumulado. Se usa al reanudar tras una pausa larga
    /// (minimizar la ventana, un breakpoint): no queremos que la simulacion se
    /// ponga al dia de golpe al volver.
    void Reset();

    void SetMaxFrameSeconds(double seconds);
    void SetMaxStepsPerFrame(int steps);
    [[nodiscard]] int MaxStepsPerFrame() const { return maxStepsPerFrame_; }

    /// Si el ultimo Advance descarto tiempo por ir demasiado retrasado. Es la
    /// senal de que la maquina no llega: si aparece de forma sostenida, hay que
    /// bajar calidad o subir el paso fijo.
    [[nodiscard]] bool DroppedTime() const { return droppedTime_; }

private:
    double fixedDelta_;
    double accumulator_ = 0.0;
    double maxFrameSeconds_ = 0.25;
    int maxStepsPerFrame_ = 8;
    bool droppedTime_ = false;
};

}  // namespace vs
