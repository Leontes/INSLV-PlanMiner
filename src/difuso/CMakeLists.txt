# difuso: la lógica difusa. Las etiquetas trapezoidales (fuzzy_t), el dominio de una variable
# (domain_t), las variables (variable_t) y el conjunto de variables (VectorVar).
inslv_anadir_sublibreria(difuso HEREDADA VectorVar.cpp domain_t.cpp fuzzy_t.cpp variable_t.cpp)
# genetico: VectorVar.h incluye genetcode.h (adaptación de un ejemplo a una regla codificada).
target_link_libraries(inslv_difuso PUBLIC inslv_datos inslv_genetico)
# GCC avisa al optimizar de tres sprintf en un char[10] (domain_t.cpp), que solo se desbordarían
# con más de 10^8 etiquetas: se corrigen al modernizarlo.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(inslv_difuso PRIVATE -Wno-format-overflow)
endif()
