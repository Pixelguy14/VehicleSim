#include <cmath>
#include <limits>

#include <doctest/doctest.h>

#include "core/FixedStepper.h"

TEST_CASE("FixedStepper acumula tiempo hasta completar un paso") {
    vs::FixedStepper stepper;

    CHECK(stepper.Advance(0.0) == 0);
    CHECK(stepper.Advance(1.0 / 120.0) == 0);
    CHECK(stepper.Alpha() == doctest::Approx(0.5).epsilon(1e-3));

    CHECK(stepper.Advance(1.0 / 120.0) == 1);
    CHECK(stepper.Alpha() == doctest::Approx(0.0).epsilon(1e-9));
}

TEST_CASE("FixedStepper ejecuta varios pasos en un frame atrasado") {
    vs::FixedStepper stepper;

    // Un frame de 50 ms son tres pasos de 1/60.
    CHECK(stepper.Advance(3.0 / 60.0) == 3);
}

TEST_CASE("FixedStepper acota un frame gigante en lugar de simular de golpe") {
    vs::FixedStepper stepper;

    // Cinco segundos de reloj son 300 pasos: simularlos de golpe congelaria el
    // juego. Se acota el tiempo de entrada y se avisa del descarte.
    const int steps = stepper.Advance(5.0);
    CHECK(steps <= 8);
    CHECK(stepper.DroppedTime());
    CHECK(stepper.AccumulatorSeconds() < stepper.FixedDelta());

    // El frame siguiente vuelve a la normalidad: no arrastra la deuda.
    CHECK(stepper.Advance(1.0 / 60.0) == 1);
    CHECK_FALSE(stepper.DroppedTime());
}

TEST_CASE("FixedStepper respeta el presupuesto de pasos por frame") {
    vs::FixedStepper stepper;
    stepper.SetMaxStepsPerFrame(2);

    CHECK(stepper.Advance(10.0 / 60.0) == 2);
    CHECK(stepper.DroppedTime());
    CHECK(stepper.AccumulatorSeconds() < stepper.FixedDelta());
}

TEST_CASE("FixedStepper ignora tiempos invalidos") {
    vs::FixedStepper stepper;

    CHECK(stepper.Advance(-1.0) == 0);
    CHECK(stepper.Advance(std::nan("")) == 0);
    CHECK(stepper.Advance(std::numeric_limits<double>::quiet_NaN()) == 0);
    CHECK(stepper.Alpha() == doctest::Approx(0.0));
}

TEST_CASE("FixedStepper conserva el tiempo real a cualquier refresco") {
    // La invariante: sin descartes, el tiempo consumido por los pasos mas el que
    // queda en el acumulador tiene que ser exactamente el tiempo de reloj. Si no
    // se cumple, la simulacion adelanta o atrasa respecto al mundo real.
    const double rates[] = {60.0, 120.0, 144.0, 59.94, 165.0};

    for (const double rate : rates) {
        CAPTURE(rate);

        vs::FixedStepper stepper;
        const double frame = 1.0 / rate;
        double steps = 0.0;
        bool dropped = false;

        for (int i = 0; i < 600; ++i) {
            steps += static_cast<double>(stepper.Advance(frame));
            dropped = dropped || stepper.DroppedTime();
        }

        const double simulated = steps * stepper.FixedDelta() + stepper.AccumulatorSeconds();

        CHECK_FALSE(dropped);
        CHECK(simulated == doctest::Approx(600.0 * frame).epsilon(1e-9));
    }
}

TEST_CASE("FixedStepper reparte el tiempo distinto segun el refresco, pero no lo pierde") {
    // Sumar 144 veces 1/144 no da 1.0 exacto, asi que el paso 60 puede caer en el
    // frame 145. Es redondeo binario, no un bug: el tiempo sigue ahi y se consume
    // al frame siguiente. Lo que se comprueba es que la diferencia nunca pasa de
    // un paso, porque entonces si seria un desfase real.
    vs::FixedStepper at120;
    vs::FixedStepper at144;

    double steps120 = 0.0;
    double steps144 = 0.0;
    for (int i = 0; i < 120; ++i) {
        steps120 += static_cast<double>(at120.Advance(1.0 / 120.0));
    }
    for (int i = 0; i < 144; ++i) {
        steps144 += static_cast<double>(at144.Advance(1.0 / 144.0));
    }

    CHECK(steps120 == doctest::Approx(60.0));
    CHECK(std::fabs(steps120 - steps144) <= 1.0);
}

TEST_CASE("FixedStepper Reset descarta el tiempo pendiente") {
    vs::FixedStepper stepper;

    stepper.Advance(0.4);
    stepper.Reset();

    CHECK(stepper.AccumulatorSeconds() == doctest::Approx(0.0));
    CHECK(stepper.Alpha() == doctest::Approx(0.0));
    CHECK_FALSE(stepper.DroppedTime());
}
