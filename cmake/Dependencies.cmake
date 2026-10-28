# Dependencias externas.
#
# Todas se fijan por etiqueta exacta, nunca por rama ni por HEAD: una rama
# cambia y el build deja de ser reproducible, que es justo lo que necesitamos
# para el Sprint 6.
#
# La primera configuracion necesita red. Despues, las fuentes quedan en
# build/<preset>/_deps y no se vuelven a descargar.

include(FetchContent)

# --- GLFW: ventana, contexto OpenGL y entrada ---------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4  # a74efa0d5628b74adc0426af4c5710e287fa7c2c
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
)

# --- Box2D v3: motor de cuerpos rigidos --------------------------------------
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_PROFILE OFF CACHE BOOL "" FORCE)
set(BOX2D_COMPILE_WARNING_AS_ERROR OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.1  # 8c661469c9507d3ad6fbd2fea3f1aa71669c2fe3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
)

# --- doctest: framework de tests ----------------------------------------------
# Todavia no se usa: entra en el Sprint 0.8. Se declara aqui para que el
# primer configure ya lo descargue y el 0.8 no dependa de la red.
#
# SOURCE_SUBDIR apunta a una carpeta que no existe a proposito: asi CMake
# descarga las fuentes pero no configura el CMakeLists de doctest, que pide
# cmake_minimum_required 3.0 y ensucia cada configure con un warning de
# deprecacion. Como la biblioteca es header-only, no lo necesitamos: mas abajo
# definimos nuestro propio target de interfaz.
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11  # ae7a13539fb71f270b87eb2e874fbac80bc8dda2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    SOURCE_SUBDIR cmake-not-used
    EXCLUDE_FROM_ALL
)

# --- Dear ImGui: superposicion de depuracion ---------------------------------
# Se descarga el repositorio y se compilan a mano solo los archivos que hacen
# falta. ImGui no trae CMakeLists: su build oficial son proyectos de Visual
# Studio y Makefiles propios, asi que el target se define aqui.
#
# Fijado por etiqueta, como el resto: un cambio en la rama master de ImGui
# puede reordenar la API de widgets y romper el build sin que nadie haya tocado
# este repositorio.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(glfw box2d doctest imgui)

add_library(vs_doctest INTERFACE)
add_library(doctest::doctest ALIAS vs_doctest)

target_include_directories(vs_doctest
    SYSTEM INTERFACE
        "${doctest_SOURCE_DIR}"
)

# --- glad: vendorizado, no se descarga ---------------------------------------
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/third_party/glad"
    "${CMAKE_BINARY_DIR}/third_party/glad"
)

# Las cabeceras de dependencias se tratan como system: sus warnings no son
# nuestros y no deben romper el build con -Werror. Box2D, por ejemplo, usa
# casts estilo C en sus cabeceras y activa -Wold-style-cast y
# -Wdouble-promotion. Con la propiedad SYSTEM, el compilador aplica sus reglas
# pero silencia los diagnosticos de esos archivos.
foreach(vs_dependency glfw box2d glad)
    if(TARGET ${vs_dependency})
        set_property(TARGET ${vs_dependency} PROPERTY SYSTEM TRUE)
    endif()
endforeach()

# --- Dear ImGui: se compila aqui, no se usa su build oficial ------------------
# Solo los cuatro archivos del nucleo y los dos backends que usamos. ImGui trae
# decenas de ejemplos y backends para Vulkan, DirectX, SDL y demas; compilarlos
# todos multiplicaria el tiempo de build sin aportar nada.
add_library(vs_imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(vs_imgui
    SYSTEM PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
)

# Aviso sobre el cargador de OpenGL: ImGui 1.92 ya no acepta un cargador
# externo. Define IMGUI_IMPL_OPENGL_LOADER_CUSTOM para *saltarse* su cargador
# embebido, pero no incluye ninguna cabecera en su lugar, asi que definirlo sin
# mas deja el backend sin simbolos GL. Se usa su cargador (gl3w recortado): vive
# en su propia unidad de traduccion, con simbolos renombrados, y no colisiona
# con glad.
target_link_libraries(vs_imgui
    PUBLIC
        glfw
)

set_property(TARGET vs_imgui PROPERTY SYSTEM TRUE)

