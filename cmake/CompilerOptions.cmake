# Opciones de compilacion que afectan al comportamiento del binario, no solo a
# los mensajes del compilador. Aqui viven dos decisiones del proyecto:
#
# 1. Runtime estatico de MSVC (/MT). El .zip portable de Windows tiene que
#    funcionar sin instalar el VC++ Redistributable en la maquina destino.
#    Debe fijarse antes de crear cualquier target, porque CMake lo lee en el
#    momento de crear el target.
#
# 2. Contraccion FMA desactivada (-ffp-contract=off, /fp:precise). El
#    compilador puede fusionar a*b+c en una sola instruccion FMA, que redondea
#    una vez en lugar de dos. Eso produce resultados distintos entre
#    plataformas aunque el codigo sea identico. Es una de las fuentes de
#    divergencia numerica que documenta el ADR-0002.

if(MSVC)
    if(VS_STATIC_RUNTIME)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            CACHE STRING "Runtime de MSVC" FORCE)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
            CACHE STRING "Runtime de MSVC" FORCE)
    endif()
endif()

set(VS_CLANG_GCC_OPTIONS
    -ffp-contract=off
    -fno-fast-math
)

set(VS_MSVC_OPTIONS
    /EHsc
    /fp:precise
    /permissive-
    /utf-8
    /Zc:__cplusplus
    /Zc:preprocessor
    /MP
)

function(vs_apply_project_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE ${VS_MSVC_OPTIONS})
    else()
        target_compile_options(${target} PRIVATE ${VS_CLANG_GCC_OPTIONS})
    endif()

    # macOS declara OpenGL obsoleto desde 10.14: cada llamada a gl*() avisa, y
    # con -Werror el proyecto no compila en Mac. La definicion silencia ese
    # aviso concreto, no todos. No hay plan de migrar a Metal en este proyecto
    # (el objetivo es un juego 2D y GL 3.3 funciona en las tres plataformas),
    # asi que el aviso no aporta nada util aqui.
    if(APPLE)
        target_compile_definitions(${target} PRIVATE GL_SILENCE_DEPRECATION)
    endif()
endfunction()
