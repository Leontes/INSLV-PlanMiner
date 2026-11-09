inslv_anadir_tests(inslv_unidad unidad test_aprender.cpp test_nslv.cpp test_opciones.cpp test_verbosidad.cpp
                   test_clasificar.cpp
                   test_arreglos.cpp
                   test_conjunto_de_reglas.cpp
                   test_tabla.cpp
                   test_patrones.cpp
                   test_bondad.cpp
                   test_generador_de_azar.cpp
                   test_caracteristicas.cpp)
target_link_libraries(inslv_unidad PRIVATE inslv_apoyo)
