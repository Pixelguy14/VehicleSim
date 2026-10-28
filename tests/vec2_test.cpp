#include <doctest/doctest.h>

#include "Near.h"
#include "math/Angles.h"
#include "math/Vec2.h"

TEST_CASE("Vec2: suma, resta y escala") {
    const vs::Vec2 a{1.0F, 2.0F};
    const vs::Vec2 b{3.0F, -1.0F};

    CHECK(a + b == vs::Vec2{4.0F, 1.0F});
    CHECK(a - b == vs::Vec2{-2.0F, 3.0F});
    CHECK(-a == vs::Vec2{-1.0F, -2.0F});
    CHECK(a * 2.0F == vs::Vec2{2.0F, 4.0F});
    CHECK(2.0F * a == vs::Vec2{2.0F, 4.0F});
    CHECK(a / 2.0F == vs::Vec2{0.5F, 1.0F});

    vs::Vec2 c = a;
    c += b;
    CHECK(c == vs::Vec2{4.0F, 1.0F});
    c -= b;
    CHECK(c == a);
    c *= 3.0F;
    CHECK(c == vs::Vec2{3.0F, 6.0F});
}

TEST_CASE("Vec2: producto punto y producto cruz") {
    CHECK(vs::Dot({1.0F, 0.0F}, {0.0F, 1.0F}) == vstest::Near(0.0F));
    CHECK(vs::Dot({2.0F, 3.0F}, {4.0F, 5.0F}) == vstest::Near(23.0F));

    // El signo del producto cruz dice de que lado esta el otro vector: es la
    // operacion con la que se decide si un coche esta a la izquierda o a la
    // derecha de una calle, sin hacer trigonometria.
    CHECK(vs::Cross({1.0F, 0.0F}, {0.0F, 1.0F}) == vstest::Near(1.0F));
    CHECK(vs::Cross({0.0F, 1.0F}, {1.0F, 0.0F}) == vstest::Near(-1.0F));
    CHECK(vs::Cross({1.0F, 0.0F}, {2.0F, 0.0F}) == vstest::Near(0.0F));
}

TEST_CASE("Vec2: longitud y normalizacion") {
    CHECK(vs::Length({3.0F, 4.0F}) == vstest::Near(5.0F));
    CHECK(vs::LengthSquared({3.0F, 4.0F}) == vstest::Near(25.0F));

    const vs::Vec2 unit = vs::Normalized({3.0F, 4.0F});
    CHECK(vs::Length(unit) == vstest::Near(1.0F));
    CHECK(unit.x == vstest::Near(0.6F));

    // Normalizar el vector nulo devuelve cero en lugar de NaN, que es lo que
    // aparece si se divide por la longitud sin comprobarla.
    CHECK(vs::Normalized({0.0F, 0.0F}) == vs::Vec2{0.0F, 0.0F});
}

TEST_CASE("Vec2: rotacion e interpolacion") {
    const vs::Vec2 rotated = vs::Rotated({1.0F, 0.0F}, vs::kPi / 2.0F);
    CHECK(rotated.x == vstest::Near(0.0F));
    CHECK(rotated.y == vstest::Near(1.0F));

    // Interpolar puntos es seguro: no hay vueltas de 360 grados que valgan.
    const vs::Vec2 middle = vs::Lerp({0.0F, 0.0F}, {10.0F, 20.0F}, 0.5F);
    CHECK(middle == vs::Vec2{5.0F, 10.0F});
    CHECK(vs::Lerp({1.0F, 1.0F}, {2.0F, 2.0F}, 0.0F) == vs::Vec2{1.0F, 1.0F});
    CHECK(vs::Lerp({1.0F, 1.0F}, {2.0F, 2.0F}, 1.0F) == vs::Vec2{2.0F, 2.0F});
}

TEST_CASE("Vec2: el angulo de un vector usa el convenio del mundo") {
    CHECK(vs::Angle({1.0F, 0.0F}) == vstest::Near(0.0F));
    CHECK(vs::Angle({0.0F, 1.0F}) == vstest::Near(vs::kPi / 2.0F));
}
