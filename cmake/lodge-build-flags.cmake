#
# A target that can be used to inherit platform specific default build flags.
#
add_library(lodge-build-flags INTERFACE)

target_compile_features(lodge-build-flags
    INTERFACE
        c_std_11
        c_variadic_macros
)

#
# Clang build flags.
#
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(lodge-build-flags
        INTERFACE
            -Wall                                   # More warnings
            -fdiagnostics-show-option               # More warnings
            -Werror-implicit-function-declaration   # Do not allow declaring functions without implementing them.
            #-Werror=strict-aliasing                 # Force casting of pointer types. (DISABLED BECAUSE OF STB.H)
            -Werror=return-type                     # Force correct return type.
			-Wno-missing-braces						# Missing braces warning is bugged when nested type has { 0 }
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    # DEBUG: Bounds checking.
    #target_compile_options(lodge-build-flags
    #    INTERFACE
    #        -fsanitize=address
    #)
    #target_link_options(lodge-build-flags
    #    INTERFACE
    #        -fsanitize=address
    #)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(lodge-build-flags
        INTERFACE
            "_CRT_SECURE_NO_WARNINGS"       # Disable "unsafe" functions that redirects to Microsoft-specific implementations.
    )

    target_compile_options(lodge-build-flags
        INTERFACE
            "/W3"                           # More warnings.
            "/we4020"                       # 'function': too many actual parameters
            "/we4022"                       # 'function' : pointer mismatch for actual parameter 'number'
            "/we4098"                       # 'function': 'void' function returning a value
            "/we4047"                       # "differs in levels of indirection" (trying to pass ptr** instead of ptr*)
            "/we4024"                       # 'function' : different types for formal and actual parameter 'number'
            "/we4029"                       # declared formal parameter list different from declaration
            "/we4715"                       # 'function' : not all control paths return a value
            "/we4716"                       # 'function': must return a value
            "/we4028"                       # formal parameter 1 different from declaration
            "/we4090"                       # 'initializing': different 'const' qualifiers
            #"/we4244"                       # 'function': conversion from 'int' to 'const float', possible loss of data
            "/we4013"                       # 'symbol' undefined; assuming extern returning int
            "/we4133"                       # 'initializing|function': incompatible types - from '<type> *' to '<other_type> *'
            "/we4101"                       # 'identifier' : unreferenced local variable
            "/we4477"                       # 'printf' : format string '%.*s' requires an argument of type 'int', but variadic argument 1 has type 'uintptr_t'
            "/we4473"                       # not enough arguments passed for format string
    )
endif()
