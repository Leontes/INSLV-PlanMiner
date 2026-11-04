# reglas: el conjunto de reglas (ConjuntoDeReglas), con su inferencia.
inslv_anadir_sublibreria(reglas conjunto_de_reglas.cpp)
target_link_libraries(inslv_reglas PUBLIC inslv_datos inslv_difuso inslv_genetico)
