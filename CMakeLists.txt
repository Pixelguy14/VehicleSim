cmake_minimum_required(VERSION 3.28)

project(
    vehicle_sim
    VERSION 0.1.0
    DESCRIPTION "Simulacion 2D de trafico urbano con fisica de vehiculos e IA de enjambre"
    LANGUAGES C CXX
)

include(GNUInstallDirs)

# --- Opciones del proyecto ---------------------------------------------------
# Deben existir antes de incluir cmake/CompilerOptions.cmake, que las consulta.
option(VS_WARNINGS_AS_ERRORS "Trata los warnings del compilador como errores" OFF)
option(VS_STATIC_RUNTIME "Enlaza el runtime de MSVC estaticamente (/MT)" ON)

# --- Configuracion global ----------------------------------------------------
# Se fija antes de configurar dependencias: sus propios targets heredan estos
# valores y no queremos que el estandar cambie segun el subproyecto.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Si alguien configura sin preset y sin build type, elegimos uno razonable en
# lugar de compilar sin optimizar ni comprobar nada.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Sin CMAKE_BUILD_TYPE: usando RelWithDebInfo")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Tipo de build" FORCE)
endif()

# --- Modulos del proyecto ----------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerOptions)
include(CompilerWarnings)
include(Dependencies)

# --- Tests -------------------------------------------------------------------
# include(CTest) define BUILD_TESTING (ON por defecto) y habilita `ctest`. Va
# antes de add_subdirectory(src) porque src/CMakeLists tambien registra tests
# (la prueba de humo --selftest) y necesita ver la opcion ya definida.
include(CTest)

add_subdirectory(src)

# Al empaquetar se puede pasar -DBUILD_TESTING=OFF para no compilarlos, pero en
# desarrollo y en integracion continua queremos que esten siempre.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
