# aprendizaje: el aprendizaje incremental de NSLV. El algoritmo por episodios (aprender), el genético
# que busca cada regla (busqueda), la bondad de un individuo (bondad), los patrones con los que se
# combinan las reglas (patrones), la tabla de adaptaciones (tabla), las medidas de información
# (medidas) y las reglas características (caracteristicas).
inslv_anadir_sublibreria(aprendizaje aprender.cpp bondad.cpp busqueda.cpp caracteristicas.cpp
                         medidas.cpp patrones.cpp tabla.cpp)
target_link_libraries(inslv_aprendizaje PUBLIC inslv_datos inslv_difuso inslv_genetico
                                               inslv_reglas)
