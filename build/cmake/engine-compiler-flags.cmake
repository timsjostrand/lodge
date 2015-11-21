#
# Sets a variable ENGINE_COMPILER_FLAGS that can be used by targets to comply
# with the engine projects warning levels:
#
# set_property(TARGET the_target PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
#

# GCC compiler settings.
if(CMAKE_COMPILER_IS_GNUCC)
    # More warnings.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Wall -Wpedantic -fdiagnostics-show-option")
    # Do not allow declaring functions without implementing them.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Werror-implicit-function-declaration")
    # Force casting of pointer types.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Werror=strict-aliasing")
    # Force correct return type.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Werror=return-type")
endif()

# MSVC compiler settings.
if(MSVC)
    # More warnings.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /W3")
    # Disable "unsafe" functions that redirects to Microsoft-specific
    # implementations.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /D_CRT_SECURE_NO_WARNINGS")
    # Disable warning C4996: 'x': This function or variable may be unsafe. Consider using 'x_s' instead.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /wd4996")
    # Enable error C4020: 'function': too many actual parameters
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /we4020")
    # Enable error C4022: 'function' : pointer mismatch for actual parameter 'number'
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /we4022")
    # Enable error CC4098: 'function': 'void' function returning a value
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /we4098")
    # Enable error C4024: 'function' : different types for formal and actual parameter 'number'
    #set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} /we4024")
endif()

# Clang compiler settings.
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    # DEBUG: Bounds checking.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -fsanitize=address")
    set(ENGINE_LINKER_FLAGS "${ENGINE_LINKER_FLAGS} -fsanitize=address")
endif()
