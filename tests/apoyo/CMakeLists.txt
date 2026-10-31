# Lo que comparten los tests: la lectura de los casos de PlanMiner y de las reglas de referencia,
# y una lectura mínima del formato de KEEL. INSLV_DATOS: tests/datos.

add_library(inslv_apoyo STATIC casos.cpp keel.cpp referencias.cpp)
target_include_directories(inslv_apoyo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(inslv_apoyo PUBLIC inslv::inslv)
target_compile_features(inslv_apoyo PUBLIC cxx_std_20)
target_compile_definitions(
  inslv_apoyo
  PUBLIC INSLV_DATOS="${PROJECT_SOURCE_DIR}/tests/datos"
  PRIVATE INSLV_CASOS_PLANMINER="${PROJECT_SOURCE_DIR}/tests/datos/planminer")
inslv_activar_avisos(inslv_apoyo)
inslv_activar_sanitizers(inslv_apoyo)
