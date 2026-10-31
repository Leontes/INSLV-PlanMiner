# difuso: la lógica difusa. Las etiquetas trapezoidales (fuzzy_t), el dominio de una variable
# (domain_t), las variables (variable_t) y el conjunto de variables (VectorVar).
inslv_anadir_sublibreria(difuso HEREDADA VectorVar.cpp domain_t.cpp fuzzy_t.cpp variable_t.cpp)
target_link_libraries(inslv_difuso PUBLIC inslv_datos)
