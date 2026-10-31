# datos: los ejemplos (example_set) y el vector de valores de un ejemplo (vectordouble).
inslv_anadir_sublibreria(datos HEREDADA example_set.cpp vectordouble.cpp)
target_link_libraries(inslv_datos PUBLIC inslv_comun)
