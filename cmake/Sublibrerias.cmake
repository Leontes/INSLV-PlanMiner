# Las sublibrerías de INSLV: una por cada parte de la implementación.
#
# Cada sublibrería tiene sus fuentes en src/<nombre> y sus cabeceras en include/inslv/<nombre>,
# que se incluyen como "inslv/<nombre>/<cabecera>.h". Las dependencias entre sublibrerías se
# declaran con target_link_libraries en su CMakeLists.txt.

# inslv_anadir_sublibreria(<nombre> [HEREDADA] <fuentes>...): la librería estática
# inslv_<nombre>, con C++20, sin contracción de operaciones en coma flotante (las reglas tienen
# que salir bit a bit iguales que las del original) y con los sanitizers y la cobertura si se
# piden.
#
# HEREDADA: código copiado de old/ que aún no se ha modernizado. Se compila sin los avisos del
# código nuevo (traía 418 con -Wall -Wextra) y sin clang-tidy, pero con -Werror=return-type: un
# return que falte es lo que hacía fallar el original con los compiladores actuales. No vale -w,
# que también lo calla. Sin HEREDADA, los avisos y clang-tidy del código nuevo.
function(inslv_anadir_sublibreria nombre)
  cmake_parse_arguments(PARSE_ARGV 1 arg "HEREDADA" "" "")
  set(target inslv_${nombre})

  add_library(${target} STATIC ${arg_UNPARSED_ARGUMENTS})
  target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/include")
  target_compile_features(${target} PUBLIC cxx_std_20)
  target_compile_options(${target} PRIVATE -ffp-contract=off)
  inslv_activar_sanitizers(${target})
  inslv_activar_cobertura(${target})

  if(arg_HEREDADA)
    # Clang da por defecto avisos que GCC no da (arrays de longitud variable, using namespace std
    # antes de las cabeceras estándar...): se callan todos.
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(${target} PRIVATE -Wno-everything)
    endif()
    target_compile_options(${target} PRIVATE -Werror=return-type)
    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "")
  else()
    inslv_activar_avisos(${target})
  endif()
endfunction()
