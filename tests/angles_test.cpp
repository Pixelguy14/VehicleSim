#include <cmath>
#include <limits>

#include <doctest/doctest.h>

#include "Near.h"
#include "math/Angles.h"

TEST_CASE("WrapAngle acota cualquier angulo a (-pi, pi]") {
    CHECK(vs::WrapAngle(0.0F) == vstest::Near(0.0F));
    CHECK(vs::WrapAngle(vs::kPi) == vstest::Near(-vs::kPi));
    CHECK(vs::WrapAngle(1.5F * vs::kPi) == vstest::Near(-0.5F * vs::kPi));
    CHECK(vs::WrapAngle(-1.5F * vs::kPi) == vstest::Near(0.5F * vs::kPi));
    CHECK(vs::WrapAngle(0.25F * vs::kPi) == vstest::Near(0.25F * vs::kPi));

    // Diez vueltas completas vuelven al origen: es lo que evita que un angulo
    // que crece sin parar pierda precision con el tiempo.
    CHECK(vs::WrapAngle(20.0F * vs::kPi) == vstest::Near(0.0F).epsilon(1e-3));
}

TEST_CASE("WrapAngle descarta valores no finitos") {
    // Un NaN que se cuele en un angulo contamina toda la transformacion y el
    // objeto desaparece de la pantalla sin mensaje de error. Mejor cero.
    CHECK(vs::WrapAngle(std::numeric_limits<float>::infinity()) == vstest::Near(0.0F));
    CHECK(vs::WrapAngle(-std::numeric_limits<float>::infinity()) == vstest::Near(0.0F));
    CHECK(vs::WrapAngle(std::nanf("")) == vstest::Near(0.0F));
}

TEST_CASE("ShortestAngleDelta devuelve el camino corto") {
    CHECK(vs::ShortestAngleDelta(0.0F, vs::kPi / 2.0F) == vstest::Near(vs::kPi / 2.0F));

    // De 350 a 10 grados la distancia corta son 20 grados hacia delante, no 340
    // hacia atras.
    const float from = 350.0F * vs::kPi / 180.0F;
    const float to = 10.0F * vs::kPi / 180.0F;
    CHECK(vs::ShortestAngleDelta(from, to) == vstest::Near(20.0F * vs::kPi / 180.0F));

    CHECK(vs::ShortestAngleDelta(1.0F, 1.0F) == vstest::Near(0.0F));
}

TEST_CASE("AngleLerp no da la vuelta larga al cruzar el eje") {
    const float from = 350.0F * vs::kPi / 180.0F;
    const float to = 10.0F * vs::kPi / 180.0F;

    // El punto medio debe ser 0 grados (o 360, que es el mismo angulo), y no
    // 180, que es lo que daria la interpolacion ingenua.
    const float middle = vs::AngleLerp(from, to, 0.5F);
    CHECK(std::fabs(vs::WrapAngle(middle)) == vstest::Near(0.0F));

    SUBCASE("los extremos son exactos") {
        CHECK(vs::AngleLerp(1.0F, 2.0F, 0.0F) == vstest::Near(1.0F));
        CHECK(vs::AngleLerp(1.0F, 2.0F, 1.0F) == vstest::Near(2.0F));
    }

    SUBCASE("un cuarto de vuelta se reparte a la mitad") {
        CHECK(vs::AngleLerp(0.0F, vs::kPi / 2.0F, 0.5F) ==
              vstest::Near(vs::kPi / 4.0F));
    }
}
