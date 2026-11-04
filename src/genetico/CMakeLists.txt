# genetico: el algoritmo genético. El código genético de un individuo (genetcode, solo cabecera),
# las poblaciones binaria, entera y real (con lo común en poblacion), la multipoblación que las
# junta y el azar que comparten (operaciones).
inslv_anadir_sublibreria(
  genetico
  multipoblacion.cpp
  operaciones.cpp
  poblacion.cpp
  poblacionbinaria.cpp
  poblacionentera.cpp
  poblacionreal.cpp)
target_link_libraries(inslv_genetico PUBLIC inslv_comun)
