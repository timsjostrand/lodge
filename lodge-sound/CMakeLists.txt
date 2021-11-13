# Library: OpenAL.
if(NOT APPLE)
    # MinGW time.h defines 'struct timespec'
    if(MINGW)
        add_definitions(-D_TIMESPEC_DEFINED)
    endif()

    # Build OpenAL-Soft.
    if(ENABLE_SHARED)
        set(LIBTYPE "SHARED" CACHE STRING "Build static library" FORCE)
    else()
        set(LIBTYPE "STATIC" CACHE STRING "Build static library" FORCE)
    endif()
    set(ALSOFT_UTILS OFF CACHE BOOL "Build and install utility programs" FORCE)
    set(ALSOFT_NO_CONFIG_UTIL OFF CACHE BOOL "Disable building the alsoft-config utility" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "Build and install example programs" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "Build and install test programs" FORCE)
    set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "Install alsoft.conf sample configuration file" FORCE)
    set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "Install HRTF definition files" FORCE)
    set(ALSOFT_INSTALL OFF CACHE BOOL "Install headers and libraries" FORCE)
    # .lib-generation requires GNU 'sed' but MinGW usually ships with old version.
    set(ALSOFT_BUILD_IMPORT_LIB OFF CACHE BOOL "Build an import .lib using dlltool (requires sed)" FORCE)
    add_subdirectory(lib/openal ${CMAKE_BINARY_DIR}/lib/openal)
    set(OPENAL_INCLUDE_DIR lib/openal/include/AL)

    set(OPENAL_LIBRARY OpenAL)
else()
    # Use the OS X OpenAL framework.
    include(FindOpenAL)
endif()

# lodge-sound
add_library(lodge-sound STATIC "")

target_sources(lodge-sound
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/sound.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/sound.h"
)

target_include_directories(lodge-sound
    PRIVATE
        "lib/"
        "${OPENAL_INCLUDE_DIR}"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(lodge-sound
    PRIVATE
        lodge-build-flags
        ${OPENAL_LIBRARY}
        lodge-stb
    PUBLIC
        lodge-collections
        lodge-lib
)
