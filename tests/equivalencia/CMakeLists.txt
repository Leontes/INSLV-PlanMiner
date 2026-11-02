# Los casos de PlanMiner y las reglas de referencia rápidas (equivalencia), y las de referencia
# lentas (lento), en el mismo ejecutable.
inslv_compilar_tests(inslv_equivalencia test_equivalencia.cpp test_referencias.cpp)
target_link_libraries(inslv_equivalencia PRIVATE inslv_apoyo)
inslv_registrar_tests(inslv_equivalencia equivalencia FILTRO "-Lentas/*")
inslv_registrar_tests(inslv_equivalencia lento FILTRO "Lentas/*")
