# La librería INSLV: la interfaz pública (include/inslv/inslv.hpp, inslv.cpp) sobre las
# sublibrerías de la implementación, de la base a lo que la usa:
#
#   datos ← genetico ← difuso ← reglas ← aprendizaje ← inslv
#
# Todas son código heredado de old/ (HEREDADA) hasta que se modernicen.

add_subdirectory(datos)
add_subdirectory(genetico)
add_subdirectory(difuso)
add_subdirectory(reglas)
add_subdirectory(aprendizaje)

add_library(inslv STATIC inslv.cpp)
add_library(inslv::inslv ALIAS inslv)

target_include_directories(inslv PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(inslv PUBLIC cxx_std_20)
target_compile_options(inslv PRIVATE -ffp-contract=off)
# Las sublibrerías no forman parte de la interfaz: quien usa INSLV solo enlaza con inslv::inslv.
target_link_libraries(inslv PRIVATE inslv_aprendizaje)
inslv_activar_avisos(inslv)
inslv_activar_sanitizers(inslv)
