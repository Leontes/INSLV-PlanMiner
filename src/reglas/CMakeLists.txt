# reglas: el conjunto de reglas (ruleset), con su inferencia.
inslv_anadir_sublibreria(reglas ruleset.cpp)
target_link_libraries(inslv_reglas PUBLIC inslv_datos inslv_difuso inslv_genetico)
