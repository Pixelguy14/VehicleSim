#include <doctest/doctest.h>

#include "core/FrameStats.h"

TEST_CASE("FrameStats no publica nada antes de un segundo") {
    vs::FrameStats stats;
    vs::FrameSnapshot snapshot;

    CHECK_FALSE(stats.TakeSnapshot(snapshot));

    for (int i = 0; i < 60; ++i) {
        stats.AddFrame(1.0 / 120.0, 0.0005, 2);
    }
    CHECK_FALSE(stats.TakeSnapshot(snapshot));
}

TEST_CASE("FrameStats mide un segundo a 120 fps") {
    vs::FrameStats stats;
    vs::FrameSnapshot snapshot;

    for (int i = 0; i < 120; ++i) {
        stats.AddFrame(1.0 / 120.0, 0.00025, 2);
    }

    REQUIRE(stats.TakeSnapshot(snapshot));
    CHECK(snapshot.fps == doctest::Approx(120.0).epsilon(0.005));
    CHECK(snapshot.frameMs == doctest::Approx(8.333).epsilon(0.002));
    CHECK(snapshot.physicsMs == doctest::Approx(0.25).epsilon(0.005));
    CHECK(snapshot.stepsPerFrame == doctest::Approx(2.0).epsilon(0.005));
    CHECK(snapshot.frames == 120);
}

TEST_CASE("FrameStats espera exactamente un segundo pese al redondeo binario") {
    // Sumar 120 veces 1/120 en double da 0.9999999999999999, no 1.0. Sin una
    // tolerancia minima, la primera medida llegaria un frame tarde por un error
    // de redondeo, y este test lo detecta.
    vs::FrameStats stats;
    vs::FrameSnapshot snapshot;

    for (int i = 0; i < 120; ++i) {
        stats.AddFrame(1.0 / 120.0, 0.0, 0);
    }

    CHECK(stats.TakeSnapshot(snapshot));
}

TEST_CASE("FrameStats detecta un frame de 30 fps") {
    vs::FrameStats stats;
    vs::FrameSnapshot snapshot;

    for (int i = 0; i < 30; ++i) {
        stats.AddFrame(1.0 / 30.0, 0.001, 2);
    }

    REQUIRE(stats.TakeSnapshot(snapshot));
    CHECK(snapshot.fps == doctest::Approx(30.0).epsilon(0.01));
    CHECK(snapshot.frameMs == doctest::Approx(33.333).epsilon(0.002));
}

TEST_CASE("FrameStats reinicia la ventana despues de publicar") {
    vs::FrameStats stats;
    vs::FrameSnapshot snapshot;

    for (int i = 0; i < 120; ++i) {
        stats.AddFrame(1.0 / 120.0, 0.0, 2);
    }
    REQUIRE(stats.TakeSnapshot(snapshot));
    CHECK_FALSE(stats.TakeSnapshot(snapshot));
}

TEST_CASE("FrameStats ignora frames con tiempo invalido") {
    // Un frame con tiempo cero o negativo no es una medida: contarlo inflaria
    // los fps hasta el infinito.
    vs::FrameStats stats;
    vs::FrameSnapshot snapshot;

    stats.AddFrame(0.0, 0.001, 1);
    stats.AddFrame(-1.0, 0.001, 1);

    for (int i = 0; i < 120; ++i) {
        stats.AddFrame(1.0 / 120.0, 0.001, 1);
    }

    REQUIRE(stats.TakeSnapshot(snapshot));
    CHECK(snapshot.frames == 120);
    CHECK(snapshot.fps == doctest::Approx(120.0).epsilon(0.005));
}
