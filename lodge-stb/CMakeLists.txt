# lodge-stb
#
# Build stb as static library and reduce warnings.

add_library(lodge-stb STATIC "")

set(STB_PATH "${ENGINE_PATH}/lib/stb")

target_sources(lodge-stb
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/stb.c"
        "${STB_PATH}/stb_vorbis.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.c"
    PUBLIC
        "${STB_PATH}/stb.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/stb_vorbis.h"
        "${STB_PATH}/stb_image.h"
)

target_include_directories(lodge-stb
    PUBLIC
        "${ENGINE_PATH}/lib"
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
