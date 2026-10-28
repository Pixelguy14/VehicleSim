# Conjunto unico de warnings para el codigo propio del proyecto.
#
# No se aplica a dependencias de terceros: sus warnings no son nuestra
# responsabilidad y ademas no tiene sentido compilarlas con -Werror. Si MSVC
# empieza a quejarse de cabeceras de terceros en /W4, se marcan sus includes
# como SYSTEM en el paso que las introduce (Sprint 0.4).
#
# Uso: vs_apply_warnings(<target>)

set(VS_CLANG_GCC_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wcast-align
    -Wcast-qual
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpointer-arith
    -Wunused
)

set(VS_MSVC_WARNINGS
    /W4
    /external:W0
    # C4820: "N bytes de relleno anadidos tras el miembro". Salta en cualquier
    # struct con padding y nunca indica un bug: el padding lo decide el ABI, no
    # el programador. Se silencia para no llenar el build de ruido inutil.
    /wd4820
    # C4127: "la expresion condicional es constante". Las macros de asercion de
    # doctest expanden a un if con una condicion que el compilador puede plegar;
    # el aviso senala el codigo del test, no un problema real.
    /wd4127
)

function(vs_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE ${VS_MSVC_WARNINGS})
        if(VS_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE ${VS_CLANG_GCC_WARNINGS})
        if(VS_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
