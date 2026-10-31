# Cobertura de los tests con gcov (INSLV_COBERTURA, solo con GCC) y su informe con gcovr.
#
# Se instrumentan la interfaz y las sublibrerías, no los tests ni old/. Después de pasar los
# tests, el target informe_cobertura escribe el informe en <build>/cobertura/:
#
#   cmake --workflow --preset cobertura
#   cmake --build --preset cobertura --target informe_cobertura

function(inslv_activar_cobertura target)
  if(NOT INSLV_COBERTURA)
    return()
  endif()
  target_compile_options(${target} PRIVATE --coverage)
  # Pública: el ejecutable que enlace con una librería instrumentada necesita el runtime de gcov.
  target_link_options(${target} PUBLIC --coverage)
endfunction()

if(NOT INSLV_COBERTURA)
  return()
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "INSLV_COBERTURA necesita GCC (gcov)")
endif()
find_program(INSLV_GCOVR gcovr REQUIRED)

# --gcov-ignore-parse-errors: gcovr 7.0 no entiende las anotaciones de bloque de gcov a partir
# de la línea 10000 ("33:10000-block 0", sin espacio), y RuleCreator.cpp tiene más de 10000
# líneas. Solo se pierden esas anotaciones: las líneas y las ramas se leen igual.
add_custom_target(
  informe_cobertura
  COMMAND "${CMAKE_COMMAND}" -E make_directory cobertura
  COMMAND
    "${INSLV_GCOVR}" --root "${PROJECT_SOURCE_DIR}" --object-directory "${PROJECT_BINARY_DIR}"
    --filter "${PROJECT_SOURCE_DIR}/src/" --filter "${PROJECT_SOURCE_DIR}/include/"
    --gcov-ignore-parse-errors=all --html-details cobertura/index.html --txt
    cobertura/resumen.txt --print-summary
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMENT "Informe de cobertura en ${PROJECT_BINARY_DIR}/cobertura/index.html"
  VERBATIM)
