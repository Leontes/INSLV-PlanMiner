# La librería INSLV: la interfaz pública (include/inslv/inslv.hpp, inslv.cpp) sobre las
# sublibrerías de la implementación, de la base a lo que la usa:
#
#   datos
#   comun ← genetico
#   comun, datos ← difuso
#   datos, difuso, genetico ← reglas ← aprendizaje ← inslv
#
# Aparte, formatos: la lectura del formato de NSLV, que solo usa los tipos de la interfaz.

add_subdirectory(comun)
add_subdirectory(datos)
add_subdirectory(genetico)
add_subdirectory(difuso)
add_subdirectory(reglas)
add_subdirectory(aprendizaje)
add_subdirectory(formatos)

add_library(inslv STATIC inslv.cpp clasificar.cpp)
add_library(inslv::inslv ALIAS inslv)

target_include_directories(inslv PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(inslv PUBLIC cxx_std_20)
target_compile_options(inslv PRIVATE -ffp-contract=off)
# Las sublibrerías no forman parte de la interfaz: quien usa INSLV solo enlaza con inslv::inslv.
target_link_libraries(inslv PRIVATE inslv_aprendizaje inslv_comun inslv_formatos)
inslv_activar_avisos(inslv)
inslv_activar_sanitizers(inslv)
inslv_activar_cobertura(inslv)
