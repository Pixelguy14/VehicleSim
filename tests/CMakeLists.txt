# Tests unitarios del nucleo.
#
# Enlazan contra vs_core, que no depende de OpenGL ni de GLFW: corren sin
# ventana, sin GPU y sin servidor grafico. Eso es lo que permite ejecutarlos en
# un contenedor de integracion continua sin montar un Xvfb.

add_executable(vs_tests
    main.cpp
    angles_test.cpp
    camera_test.cpp
    fixed_stepper_test.cpp
    frame_stats_test.cpp
    input_state_test.cpp
    vec2_test.cpp
)

target_link_libraries(vs_tests
    PRIVATE
        vs_core
        doctest::doctest
)

vs_apply_warnings(vs_tests)
vs_apply_project_options(vs_tests)

add_test(NAME vs_tests COMMAND vs_tests)

# Si un test se cuelga (un bucle infinito en el acumulador, por ejemplo), ctest
# debe cortarlo en lugar de dejar la integracion continua bloqueada para siempre.
set_tests_properties(vs_tests PROPERTIES TIMEOUT 60)
