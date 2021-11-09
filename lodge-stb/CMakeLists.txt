# lodge-stb
#
# Build stb as static library and reduce warnings.

add_library(lodge-stb STATIC "")

target_sources(lodge-stb
    PRIVATE
        "stb.c"
        "lib/stb/stb_vorbis.c"
        "stb_image.c"
    PUBLIC
        "lib/stb/stb.h"
        "stb_vorbis.h"
        "lib/stb/stb_image.h"
)

target_include_directories(lodge-stb
    PUBLIC
        "lib/"
        "./"
)

target_link_libraries(lodge-stb
    PRIVATE
        lodge-silent-build-flags
)