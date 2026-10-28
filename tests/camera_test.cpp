#include <doctest/doctest.h>

#include "Near.h"
#include "math/Camera2D.h"

namespace {

vs::Camera2D MakeCamera() {
    vs::Camera2D camera;
    camera.SetViewport(1280, 720);
    camera.SetZoom(60.0F);
    return camera;
}

}  // namespace

TEST_CASE("Camera2D: media vista en metros") {
    const vs::Camera2D camera = MakeCamera();

    // 1280 px a 60 px/m son 21.33 m de ancho, es decir 10.67 m de media anchura.
    CHECK(camera.HalfWidth() == vstest::Near(10.6667F).epsilon(1e-4));
    CHECK(camera.HalfHeight() == vstest::Near(6.0F).epsilon(1e-4));
}

TEST_CASE("Camera2D: el centro del mundo cae en el centro de la pantalla") {
    const vs::Camera2D camera = MakeCamera();

    const vs::Vec2 screen = camera.WorldToScreen({0.0F, 0.0F});
    CHECK(screen.x == vstest::Near(640.0F).epsilon(1e-4));
    CHECK(screen.y == vstest::Near(360.0F).epsilon(1e-4));

    const vs::Vec2 world = camera.ScreenToWorld({640.0F, 360.0F});
    CHECK(world.x == vstest::Near(0.0F).epsilon(1e-4));
    CHECK(world.y == vstest::Near(0.0F).epsilon(1e-4));
}

TEST_CASE("Camera2D: la Y del mundo apunta hacia arriba y la de pantalla hacia abajo") {
    const vs::Camera2D camera = MakeCamera();

    // Un metro por encima del origen tiene que dibujarse mas arriba en pantalla,
    // es decir con una Y de pixel menor.
    const vs::Vec2 above = camera.WorldToScreen({0.0F, 1.0F});
    CHECK(above.y == vstest::Near(300.0F).epsilon(1e-4));

    const vs::Vec2 right = camera.WorldToScreen({1.0F, 0.0F});
    CHECK(right.x == vstest::Near(700.0F).epsilon(1e-4));
}

TEST_CASE("Camera2D: la conversion de ida y vuelta no pierde precision") {
    const vs::Camera2D camera = MakeCamera();

    const vs::Vec2 points[] = {{0.0F, 0.0F}, {3.5F, -2.25F}, {-10.0F, 5.75F}};
    for (const vs::Vec2 point : points) {
        const vs::Vec2 back = camera.ScreenToWorld(camera.WorldToScreen(point));
        CHECK(back.x == vstest::Near(point.x).epsilon(1e-4));
        CHECK(back.y == vstest::Near(point.y).epsilon(1e-4));
    }
}

TEST_CASE("Camera2D: mover la camara desplaza la proyeccion") {
    vs::Camera2D camera = MakeCamera();
    camera.Move({5.0F, -3.0F});

    // El punto donde esta la camara pasa a ser el centro de la pantalla.
    const vs::Vec2 screen = camera.WorldToScreen({5.0F, -3.0F});
    CHECK(screen.x == vstest::Near(640.0F).epsilon(1e-4));
    CHECK(screen.y == vstest::Near(360.0F).epsilon(1e-4));

    const auto matrix = camera.ProjectionMatrix();
    // column-major: la traslacion vive en las posiciones 12 y 13.
    CHECK(matrix[12] == vstest::Near(-5.0F / 10.6667F).epsilon(1e-4));
    CHECK(matrix[13] == vstest::Near(3.0F / 6.0F).epsilon(1e-4));
}

TEST_CASE("Camera2D: la matriz de proyeccion es ortografica y column-major") {
    const vs::Camera2D camera = MakeCamera();
    const auto matrix = camera.ProjectionMatrix();

    CHECK(matrix[0] == vstest::Near(1.0F / 10.6667F).epsilon(1e-4));
    CHECK(matrix[5] == vstest::Near(1.0F / 6.0F).epsilon(1e-4));
    CHECK(matrix[10] == vstest::Near(-1.0F));
    CHECK(matrix[15] == vstest::Near(1.0F));

    // Los terminos de perspectiva deben ser cero: si alguno dejara de serlo,
    // la imagen se deformaria con la profundidad.
    CHECK(matrix[3] == vstest::Near(0.0F));
    CHECK(matrix[7] == vstest::Near(0.0F));
    CHECK(matrix[11] == vstest::Near(0.0F));
}

TEST_CASE("Camera2D: el zoom se acota y el viewport nunca es cero") {
    vs::Camera2D camera;

    camera.SetZoom(0.0F);
    CHECK(camera.Zoom() == vstest::Near(1.0F));
    camera.SetZoom(10000.0F);
    CHECK(camera.Zoom() == vstest::Near(400.0F));

    // Una ventana minimizada reporta 0x0. Sin este suelo, la proyeccion divide
    // por cero y la pantalla se llena de NaN.
    camera.SetViewport(0, 0);
    CHECK(camera.ViewportWidth() == 1);
    CHECK(camera.ViewportHeight() == 1);
}
