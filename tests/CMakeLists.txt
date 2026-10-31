# Tests, por etiquetas de CTest (ctest -L <etiqueta>):
#
# - unidad: la interfaz y, según se porte, cada pieza de la librería.
# - equivalencia: las reglas de la librería frente a las que dio el original, guardadas en
#   tests/datos. No necesitan old/: corren también en la CI.
# - lento: lo mismo con los conjuntos grandes (aut, boxA, ring), que tardan minutos. Los presets
#   no los pasan salvo el suyo: ctest --preset lento.
# - oraculo: solo en local, la librería frente a old/ compilado en el mismo programa.

include(GoogleTest)

# Un ejecutable de GoogleTest, con los avisos y los sanitizers del código nuevo.
function(inslv_compilar_tests nombre)
  add_executable(${nombre} ${ARGN})
  target_link_libraries(${nombre} PRIVATE inslv::inslv GTest::gtest_main)
  target_compile_features(${nombre} PRIVATE cxx_std_20)
  inslv_activar_avisos(${nombre})
  inslv_activar_sanitizers(${nombre})
endfunction()

# Registra en CTest los tests de un ejecutable con una etiqueta, que va también como prefijo del
# nombre (unidad.Learn.SameSeedSameRules). FILTRO: solo los que cumplen ese filtro de GoogleTest.
function(inslv_registrar_tests nombre etiqueta)
  cmake_parse_arguments(PARSE_ARGV 2 arg "" "FILTRO" "")
  set(filtro)
  if(arg_FILTRO)
    set(filtro TEST_FILTER "${arg_FILTRO}")
  endif()
  gtest_discover_tests(
    ${nombre}
    TEST_PREFIX "${etiqueta}."
    TEST_LIST "${nombre}_${etiqueta}"
    ${filtro}
    DISCOVERY_MODE PRE_TEST NO_PRETTY_VALUES
    PROPERTIES LABELS ${etiqueta})
endfunction()

# Las dos cosas: un ejecutable con los tests de una etiqueta.
function(inslv_anadir_tests nombre etiqueta)
  inslv_compilar_tests(${nombre} ${ARGN})
  inslv_registrar_tests(${nombre} ${etiqueta})
endfunction()

add_subdirectory(apoyo)
add_subdirectory(unidad)
add_subdirectory(equivalencia)

if(INSLV_ORACULO)
  include(Oraculo)
  add_subdirectory(oraculo)
else()
  message(STATUS "Tests del oráculo desactivados (INSLV_ORACULO=OFF)")
endif()
