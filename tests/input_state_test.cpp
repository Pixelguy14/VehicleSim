#include <doctest/doctest.h>

#include "input/InputState.h"

namespace {

constexpr vs::InputAction kAction = vs::InputAction::Handbrake;

}  // namespace

TEST_CASE("Un press se entrega una sola vez aunque el paso se ejecute varias veces") {
    vs::InputState state;

    state.UpdateRaw(kAction, true);
    state.BeginStep();
    CHECK(state.WasPressed(kAction));
    CHECK(state.IsDown(kAction));

    // Segundo paso del mismo frame: el flanco ya se consumio, pero la tecla
    // sigue pulsada.
    state.BeginStep();
    CHECK_FALSE(state.WasPressed(kAction));
    CHECK(state.IsDown(kAction));
}

TEST_CASE("Un press sobrevive a un frame en el que no corre ningun paso") {
    vs::InputState state;

    // Dos lecturas de hardware seguidas, ninguna consumida todavia. El flanco
    // tiene que seguir pendiente: perderlo seria perder una pulsacion real.
    state.UpdateRaw(kAction, true);
    state.UpdateRaw(kAction, true);

    state.BeginStep();
    CHECK(state.WasPressed(kAction));
}

TEST_CASE("Dos pasos en el mismo frame: solo el primero ve el flanco") {
    vs::InputState state;

    state.UpdateRaw(kAction, true);
    state.BeginStep();
    const bool first = state.WasPressed(kAction);
    state.BeginStep();
    const bool second = state.WasPressed(kAction);

    CHECK(first);
    CHECK_FALSE(second);
}

TEST_CASE("Mantener la tecla no repite el flanco") {
    vs::InputState state;

    state.UpdateRaw(kAction, true);
    state.BeginStep();
    CHECK(state.WasPressed(kAction));

    // El hardware sigue informando "pulsada": eso no es una pulsacion nueva.
    state.UpdateRaw(kAction, true);
    state.BeginStep();
    CHECK_FALSE(state.WasPressed(kAction));
    CHECK(state.IsDown(kAction));
}

TEST_CASE("El release se detecta una sola vez") {
    vs::InputState state;

    state.UpdateRaw(kAction, true);
    state.BeginStep();

    state.UpdateRaw(kAction, false);
    state.BeginStep();
    CHECK(state.WasReleased(kAction));
    CHECK_FALSE(state.IsDown(kAction));

    state.BeginStep();
    CHECK_FALSE(state.WasReleased(kAction));
}

TEST_CASE("Un tap dentro de un mismo frame se ve completo") {
    vs::InputState state;

    // Pulsar y soltar entre dos lecturas de hardware: si se mirara el estado
    // sostenido se perderia la pulsacion por completo.
    state.UpdateRaw(kAction, true);
    state.UpdateRaw(kAction, false);
    state.BeginStep();

    CHECK(state.WasPressed(kAction));
    CHECK(state.WasReleased(kAction));
    CHECK_FALSE(state.IsDown(kAction));
}

TEST_CASE("Reset descarta lo pendiente y el estado sostenido") {
    vs::InputState state;

    state.UpdateRaw(kAction, true);
    state.Reset();
    state.BeginStep();

    CHECK_FALSE(state.WasPressed(kAction));
    CHECK_FALSE(state.IsDown(kAction));
}

TEST_CASE("Las acciones son independientes entre si") {
    vs::InputState state;

    state.UpdateRaw(vs::InputAction::Throttle, true);
    state.BeginStep();

    CHECK(state.WasPressed(vs::InputAction::Throttle));
    CHECK(state.IsDown(vs::InputAction::Throttle));

    // Ninguna otra accion debe enterarse de una pulsacion ajena: es el bug
    // clasico de indexar mal el array de estados.
    CHECK_FALSE(state.WasPressed(vs::InputAction::Brake));
    CHECK_FALSE(state.IsDown(vs::InputAction::SteerLeft));
    CHECK_FALSE(state.WasReleased(vs::InputAction::Quit));
}
