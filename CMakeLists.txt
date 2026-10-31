cmake_minimum_required(VERSION 3.25)

project(
  INSLV
  VERSION 0.1.0
  DESCRIPTION "Aprendizaje de reglas difusas con el genético de NSLV, en su versión incremental"
  HOMEPAGE_URL "https://github.com/Leontes/INSLV-PlanMiner"
  LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "Compila fuera del código fuente, por ejemplo: cmake --preset debug")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Opciones. Los tests solo se compilan por defecto si INSLV es el proyecto principal, no si otro
# lo incluye con add_subdirectory o FetchContent.
option(INSLV_TESTS "Compila los tests" ${PROJECT_IS_TOP_LEVEL})
option(INSLV_AVISOS_COMO_ERRORES "Trata los avisos del código nuevo como errores" OFF)
option(INSLV_SANITIZERS "Compila la librería y los tests con AddressSanitizer y UBSan" OFF)
option(INSLV_COBERTURA "Mide la cobertura de los tests con gcov (solo con GCC)" OFF)
# old/, la implementación original, solo está en local (no se sube): el oráculo de los tests se
# compila si está.
if(INSLV_TESTS AND EXISTS "${PROJECT_SOURCE_DIR}/old/CMakeLists.txt")
  set(_oraculo_por_defecto ON)
else()
  set(_oraculo_por_defecto OFF)
endif()
option(INSLV_ORACULO "Compila old/ como oráculo de los tests" ${_oraculo_por_defecto})

if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de compilación" FORCE)
endif()

# El estándar se fija por target (cxx_std_20) y no de forma global, para que old/ siga
# compilándose con el suyo.
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(Avisos)
include(Sanitizers)
include(Sublibrerias)
include(Cobertura)

add_subdirectory(src)

if(INSLV_TESTS)
  enable_testing()
  include(Dependencias)
  add_subdirectory(tests)
endif()
