# genetico: el algoritmo genético. El código genético de un individuo (Codigo, solo cabecera), las
# poblaciones binaria, entera y real (con lo común en Poblacion), la multipoblación que las junta y
# el azar que comparten (azar).
inslv_anadir_sublibreria(
  genetico
  azar.cpp
  multipoblacion.cpp
  poblacion.cpp
  poblacion_binaria.cpp
  poblacion_entera.cpp
  poblacion_real.cpp)
target_link_libraries(inslv_genetico PUBLIC inslv_comun)
