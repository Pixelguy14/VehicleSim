# glad2 vendorizado: cargador de funciones de OpenGL 3.3 core.
#
# Se genera una sola vez con la herramienta oficial y se commitea, para que el
# build no dependa de Python, de jinja2 ni de tener red en tiempo de
# configuracion. Regenerarlo (solo si cambia la version de OpenGL objetivo):
#
#   pip install glad2
#   python -m glad --api gl:core=3.3 --extensions "" --out-path ./third_party/glad c
#
# Nota: glad2 nombra la cabecera glad/gl.h (glad 1 usaba glad/glad.h).

add_library(glad STATIC src/gl.c)
add_library(glad::glad ALIAS glad)

target_include_directories(glad
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# En Linux y otros sistemas Unix el cargador necesita dlopen para resolver
# los simbolos de OpenGL en tiempo de ejecucion.
target_link_libraries(glad
    PUBLIC
        ${CMAKE_DL_LIBS}
)

set_target_properties(glad PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Es codigo de terceros: no se le aplican nuestros warnings ni -Werror.
