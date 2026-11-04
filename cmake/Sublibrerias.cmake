# Las sublibrerías de INSLV: una por cada parte de la implementación.
#
# Cada sublibrería tiene sus fuentes en src/<nombre> y sus cabeceras en include/inslv/<nombre>,
# que se incluyen como "inslv/<nombre>/<cabecera>.h". Las dependencias entre sublibrerías se
# declaran con target_link_libraries en su CMakeLists.txt.

# inslv_anadir_sublibreria(<nombre> <fuentes>...): la librería estática inslv_<nombre>, con C++20,
# sin contracción de operaciones en coma flotante (las reglas tienen que salir bit a bit iguales
# que las del original), con los avisos y clang-tidy del proyecto y con los sanitizers y la
# cobertura si se piden.
function(inslv_anadir_sublibreria nombre)
  set(target inslv_${nombre})

  add_library(${target} STATIC ${ARGN})
  target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/include")
  target_compile_features(${target} PUBLIC cxx_std_20)
  target_compile_options(${target} PRIVATE -ffp-contract=off)
  inslv_activar_sanitizers(${target})
  inslv_activar_cobertura(${target})
  inslv_activar_avisos(${target})
endfunction()
