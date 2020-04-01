#
# Sets a variable ENGINE_COMPILER_FLAGS that can be used by targets to comply
# with the engine projects warning levels:
#
# set_property(TARGET the_target PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
#

# GCC compiler settings.
if(CMAKE_COMPILER_IS_GNUCC)
    # More warnings.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Wall -fdiagnostics-show-option")
    # Strict ISO C.
    #set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Wpedantic")
    # Do not allow declaring functions without implementing them.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Werror-implicit-function-declaration")
    # Force casting of pointer types.
    # DISABLED BECAUSE STB.H
    #set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Werror=strict-aliasing")
    # Force correct return type.
    set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -Werror=return-type")
endif()

# MSVC compiler settings.
if(MSVC)
    set(MSVC_COMPILER_FLAGS
        "/W3"                           # More warnings.
        "/D_CRT_SECURE_NO_WARNINGS"     # Disable "unsafe" functions that redirects to Microsoft-specific implementations.
        "/wd4996"                       # 'x': This function or variable may be unsafe. Consider using 'x_s' instead.
        "/we4020"                       # 'function': too many actual parameters
        "/we4022"                       # 'function' : pointer mismatch for actual parameter 'number'
        "/we4098"                       # 'function': 'void' function returning a value
        "/we4047"                       # "differs in levels of indirection" (trying to pass ptr** instead of ptr*)
        "/we4024"                       # 'function' : different types for formal and actual parameter 'number'
        "/we4029"                       # declared formal parameter list different from declaration
        "/we4715"                       # function' : not all control paths return a value
        "/we4028"                       # formal parameter 1 different from declaration
        "/we4090"                       # 'initializing': different 'const' qualifiers
        #"/we4244"                       # 'function': conversion from 'int' to 'const float', possible loss of data
    )
    
    foreach(flag ${MSVC_COMPILER_FLAGS})
        set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} ${flag}")
    endforeach()
endif()

# Clang compiler settings.
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    # DEBUG: Bounds checking.
    #set(ENGINE_COMPILER_FLAGS "${ENGINE_COMPILER_FLAGS} -fsanitize=address")
    #set(ENGINE_LINKER_FLAGS "${ENGINE_LINKER_FLAGS} -fsanitize=address")
endif()
