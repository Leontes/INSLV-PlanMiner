# reglas: el conjunto de reglas (ruleset), con su inferencia, y las relaciones entre variables
# (Relaciones).
inslv_anadir_sublibreria(reglas HEREDADA Relaciones.cpp ruleset.cpp)
target_link_libraries(inslv_reglas PUBLIC inslv_datos inslv_difuso inslv_genetico)
