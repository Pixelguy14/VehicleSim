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

TEST_CASE("SetFixedDelta cambia el ritmo sin tocar el tiempo acumulado") {
    // Es lo que hace el slider del panel de depuracion: pasar de 60 a 120 Hz en
    // caliente. Un frame de 1/60 debe rendir dos pasos a 120 Hz y uno a 60.
    vs::FixedStepper a60;
    CHECK(a60.Advance(1.0 / 60.0) == 1);

    vs::FixedStepper a120;
    a120.SetFixedDelta(1.0 / 120.0);
    CHECK(a120.FixedDelta() == doctest::Approx(1.0 / 120.0));
    CHECK(a120.Advance(1.0 / 60.0) == 2);

    // Y al reves: un frame de 1/120 a 60 Hz no llega a un paso.
    vs::FixedStepper slow;
    slow.Advance(1.0 / 120.0);
    CHECK(slow.AccumulatorSeconds() == doctest::Approx(1.0 / 120.0));
}

TEST_CASE("SetFixedDelta no dispara una rafaga al reducir el paso") {
    // El caso que rompe el panel: se acumula medio paso a 60 Hz y se baja a
    // 240 Hz. Sin recortar el acumulador, el siguiente frame ejecutaria dos
    // pasos de golpe y el coche daria un tiron visible.
    vs::FixedStepper stepper;
    CHECK(stepper.Advance(1.0 / 120.0) == 0);
    CHECK(stepper.AccumulatorSeconds() == doctest::Approx(1.0 / 120.0));

    stepper.SetFixedDelta(1.0 / 240.0);
    CHECK(stepper.AccumulatorSeconds() < stepper.FixedDelta());

    // Un frame de 1/240 tras el cambio rinde exactamente un paso, no tres.
    CHECK(stepper.Advance(1.0 / 240.0) == 1);
}

TEST_CASE("SetFixedDelta recalcula Alpha con el paso nuevo") {
    vs::FixedStepper stepper;
    stepper.Advance(1.0 / 240.0);  // un cuarto de paso a 60 Hz

    stepper.SetFixedDelta(1.0 / 240.0);
    // El tiempo pendiente era 1/240, o sea un paso entero del nuevo ritmo; el
    // recorte lo deja en cero y alpha tiene que reflejarlo.
    CHECK(stepper.Alpha() == doctest::Approx(0.0).epsilon(1e-9));

    stepper.Advance(1.0 / 480.0);
    CHECK(stepper.Alpha() == doctest::Approx(0.5).epsilon(1e-3));
}

TEST_CASE("SetFixedDelta ignora valores que colgarian Advance") {
    vs::FixedStepper stepper;
    const double original = stepper.FixedDelta();

    stepper.SetFixedDelta(0.0);
    stepper.SetFixedDelta(-1.0);
    stepper.SetFixedDelta(std::nan(""));
    stepper.SetFixedDelta(std::numeric_limits<double>::infinity());

    CHECK(stepper.FixedDelta() == doctest::Approx(original));

    // Y sigue funcionando: un paso de 1/60 con el valor original.
    CHECK(stepper.Advance(1.0 / 60.0) == 1);
}

TEST_CASE("FixedStepper Reset descarta el tiempo pendiente") {
    vs::FixedStepper stepper;

    stepper.Advance(0.4);
    stepper.Reset();

    CHECK(stepper.AccumulatorSeconds() == doctest::Approx(0.0));
    CHECK(stepper.Alpha() == doctest::Approx(0.0));
    CHECK_FALSE(stepper.DroppedTime());
}
