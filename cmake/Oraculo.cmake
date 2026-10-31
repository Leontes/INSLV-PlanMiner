# old/: la implementación original, compilada como oráculo de los tests (target inslv_legacy).
#
# No se usa el CMakeLists.txt de old/, que define los mismos targets que la librería (inslv).
# Sus fuentes se compilan con el espacio de nombres cambiado por una macro (inslv →
# inslv_legacy), sin tocar old/, para enlazarlas en el mismo programa que la librería. Los tests
# incluyen su cabecera con tests/oraculo/inslv_legacy.hpp.

# old/ solo está en local: no se sube al repositorio.
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/old/CMakeLists.txt")
  message(FATAL_ERROR "INSLV_ORACULO necesita old/, que solo está en local")
endif()

file(GLOB _fuentes_legacy CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/old/src/*.cpp")

add_library(inslv_legacy STATIC ${_fuentes_legacy})
target_include_directories(inslv_legacy PRIVATE "${PROJECT_SOURCE_DIR}/old/include"
                                                "${PROJECT_SOURCE_DIR}/old/src")
target_compile_features(inslv_legacy PRIVATE cxx_std_17)
target_compile_definitions(
  inslv_legacy
  PRIVATE inslv=inslv_legacy
  INTERFACE INSLV_CABECERA_LEGACY="${PROJECT_SOURCE_DIR}/old/include/inslv/inslv.hpp")
# Siempre optimizado, como se generaron los casos guardados: en Debug el genético es muy lento.
# Sin avisos (no se toca) y sin contracción de operaciones en coma flotante, como la librería.
target_compile_options(inslv_legacy PRIVATE -O2 -w -ffp-contract=off)
set_target_properties(inslv_legacy PROPERTIES CXX_CLANG_TIDY "")
