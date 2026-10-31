# AddressSanitizer y UndefinedBehaviorSanitizer (INSLV_SANITIZERS).
#
# Las opciones de enlace son públicas: un ejecutable que enlace con una librería
# instrumentada necesita el runtime de los sanitizers.

function(inslv_activar_sanitizers target)
  if(NOT INSLV_SANITIZERS)
    return()
  endif()
  target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer
                                           -fno-sanitize-recover=undefined)
  target_link_options(${target} PUBLIC -fsanitize=address,undefined)
endfunction()
