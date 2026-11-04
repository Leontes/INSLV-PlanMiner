inslv_anadir_tests(inslv_unidad unidad test_aprender.cpp test_nslv.cpp test_opciones.cpp test_verbosidad.cpp
                   test_clasificar.cpp
                   test_arreglos.cpp
                   test_ruleset.cpp
                   test_tabla.cpp
                   test_patrones.cpp
                   test_bondad.cpp)
target_link_libraries(inslv_unidad PRIVATE inslv_apoyo)
