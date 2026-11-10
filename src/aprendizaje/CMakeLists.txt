# aprendizaje: el aprendizaje incremental de NSLV. El algoritmo por episodios (aprender), el genético
# que busca cada regla (busqueda), la bondad de un individuo (bondad), los patrones con los que se
# combinan las reglas (patrones), la tabla de adaptaciones (tabla), las medidas de información
# (medidas), las reglas características (caracteristicas) y las comparaciones entre atributos
# (comparaciones).
inslv_anadir_sublibreria(aprendizaje aprender.cpp bondad.cpp busqueda.cpp caracteristicas.cpp
                         comparaciones.cpp medidas.cpp patrones.cpp tabla.cpp)
target_link_libraries(inslv_aprendizaje PUBLIC inslv_datos inslv_difuso inslv_genetico
                                               inslv_reglas)
