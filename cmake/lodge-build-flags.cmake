#
# A target that can be used to inherit platform specific default build flags.
#
add_library(lodge-build-flags INTERFACE)

target_compile_features(lodge-build-flags
    INTERFACE
        c_std_11
        c_variadic_macros
)

if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    target_compile_definitions(lodge-build-flags
        INTERFACE
            "_CRT_SECURE_NO_WARNINGS"               # Disable "unsafe" functions that redirects to Microsoft-specific implementations.
    )
endif()

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
    target_compile_options(lodge-build-flags
        INTERFACE
            "/W3"           # Warning Level 3
            "/WX"           # Warnings as errors
            "/wd4244"       # '=': conversion from 'size_t' to 'uint32_t', possible loss of data
            "/wd4267"       # '=': conversion from 'size_t' to 'uint32_t', possible loss of data
            "/wd4116"       # unnamed type definition in parentheses
            "/wd4305"       # 'function': truncation from 'double' to 'const float'
            "/wd5105"       #  macro expansion producing 'defined' has undefined behavior
            "/wd4005"       #  'x': macro redefinition
    )
endif()

#
# Utility function to silence build warnings for a single source file.
#
function(lodge_set_source_silent_build_flags target)
    if(CMAKE_COMPILER_IS_GNUCC)
        set_source_files_properties(${target} PROPERTIES COMPILE_FLAGS "-w" )
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set_source_files_properties(${target} PROPERTIES COMPILE_FLAGS "-Wno-everything" )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set_source_files_properties(${target} PROPERTIES COMPILE_FLAGS "/w" )
    endif()
endfunction()

#
# Target to silence build warnings -- only used for third party targets
# that we have no control over.
#
add_library(lodge-silent-build-flags INTERFACE)

if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(lodge-silent-build-flags
        INTERFACE
            "-w"
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(lodge-silent-build-flags
        INTERFACE
            "-Wno-everything"
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(lodge-silent-build-flags
        INTERFACE
            "/w"
    )
endif()
