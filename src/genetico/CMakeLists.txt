# genetico: el algoritmo genético. El código genético de un individuo (genetcode), las
# poblaciones binaria, entera y real, la multipoblación que las junta y los operadores
# aleatorios que comparten (operaciones).
inslv_anadir_sublibreria(
  genetico
  HEREDADA
  genetcode.cpp
  multipoblacion.cpp
  operaciones.cpp
  poblacionbinaria.cpp
  poblacionentera.cpp
  poblacionreal.cpp)
target_link_libraries(inslv_genetico PUBLIC inslv_datos)
