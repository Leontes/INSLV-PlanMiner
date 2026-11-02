# datos: los ejemplos (example_set) y los valores de un ejemplo (vectordouble).
inslv_anadir_sublibreria(datos example_set.cpp vectordouble.cpp)
target_link_libraries(inslv_datos PUBLIC inslv_comun)
