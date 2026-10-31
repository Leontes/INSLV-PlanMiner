inslv_anadir_tests(inslv_oraculo oraculo test_oraculo.cpp)
target_link_libraries(inslv_oraculo PRIVATE inslv_apoyo inslv_legacy)

# El generador de las reglas de referencia (tests/datos/referencias), con old/.
add_executable(inslv_generar_referencias generar_referencias.cpp)
target_link_libraries(inslv_generar_referencias PRIVATE inslv_apoyo inslv_legacy)
target_compile_features(inslv_generar_referencias PRIVATE cxx_std_20)
inslv_activar_avisos(inslv_generar_referencias)
