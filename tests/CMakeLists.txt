# Tests, por etiquetas de CTest (ctest -L <etiqueta>):
#
# - unidad: la interfaz y, según se porte, cada pieza de la librería.
# - equivalencia: las reglas de la librería frente a las que dio el original, guardadas en
#   tests/datos. No necesitan old/: corren también en la CI.
# - oraculo: solo en local, la librería frente a old/ compilado en el mismo programa.

include(GoogleTest)

# Un ejecutable de GoogleTest con los tests de una etiqueta. Cada test se registra en CTest con
# la etiqueta como prefijo (unidad.Learn.SameSeedSameRules).
function(inslv_anadir_tests nombre etiqueta)
  add_executable(${nombre} ${ARGN})
  target_link_libraries(${nombre} PRIVATE inslv::inslv GTest::gtest_main)
  target_compile_features(${nombre} PRIVATE cxx_std_20)
  inslv_activar_avisos(${nombre})
  inslv_activar_sanitizers(${nombre})
  gtest_discover_tests(${nombre} TEST_PREFIX "${etiqueta}." DISCOVERY_MODE PRE_TEST
                       NO_PRETTY_VALUES PROPERTIES LABELS ${etiqueta})
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
