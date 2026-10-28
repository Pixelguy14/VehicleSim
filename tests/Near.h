#pragma once

#include <doctest/doctest.h>

namespace vstest {

/// Comparacion aproximada entre dos floats.
///
/// doctest::Approx solo acepta double, y el proyecto compila con
/// -Wdouble-promotion: pasar un float directamente convertiria cada comparacion
/// de los tests en un warning, y con -Werror en un fallo de build. La conversion
/// se hace aqui, una vez, de forma explicita.
[[nodiscard]] inline doctest::Approx Near(float value) {
    return doctest::Approx(static_cast<double>(value));
}

}  // namespace vstest
