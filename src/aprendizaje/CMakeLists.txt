# aprendizaje: el aprendizaje incremental de NSLV (RuleCreator: Aprender, el genético por
# episodios, las funciones de bondad y las medidas de información).
inslv_anadir_sublibreria(aprendizaje HEREDADA RuleCreator.cpp)
target_link_libraries(inslv_aprendizaje PUBLIC inslv_datos inslv_difuso inslv_genetico
                                               inslv_reglas)
