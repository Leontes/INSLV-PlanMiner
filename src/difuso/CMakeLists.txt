# difuso: la lógica difusa. Las etiquetas trapezoidales (Etiqueta), el dominio de una variable
# (Dominio), las variables (VariableDifusa) y el conjunto de variables (Variables).
inslv_anadir_sublibreria(difuso dominio.cpp etiqueta.cpp variable_difusa.cpp variables.cpp)
target_link_libraries(inslv_difuso PUBLIC inslv_comun inslv_datos)
