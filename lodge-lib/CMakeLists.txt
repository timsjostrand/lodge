# lodge-lib

add_library(lodge-lib OBJECT "")

target_sources(lodge-lib
    PRIVATE
        "src/lodge.c"
        "src/lodge_time.c"
        "src/lodge_platform.c"
        "src/lodge_hash.c"
        "src/lodge_type.c"
        "src/lodge_variant.c"
        "src/lodge_properties.c"
        "src/lodge_input.c"
        "src/lodge_quadtree.c"
        "src/lodge_noise.c"
        "src/txt.c"
        "src/strview.c"
        "src/strbuf.c"
        "src/str.c"
        "src/math4.c"
        "src/blob_cur.c"
        "src/blob.c"
        "src/color.c"
        "src/gruvbox.c"
        "src/lodge_callbacks.c"
        "src/membuf.c"
        "src/dynbuf.c"
        "src/frustum.c"
        "src/coordinate_systems.c"

        "lib/murmur3/murmur3.h"
        "lib/murmur3/murmur3.c"

        "lib/perlin-noise/src/simplexnoise1234.h"
        "lib/perlin-noise/src/simplexnoise1234.c"
    PUBLIC
        "include/lodge.h"
        "include/lodge_time.h"
        "include/lodge_assert.h"
        "include/lodge_platform.h"
        "include/lodge_hash.h"
        "include/lodge_type.h"
        "include/lodge_variant.h"
        "include/lodge_rect.h"
        "include/lodge_properties.h"
        "include/lodge_input.h"
        "include/lodge_keys.h"
        "include/lodge_quadtree.h"
        "include/lodge_noise.h"
        "include/log.h"
        "include/txt.h"
        "include/strview.h"
        "include/strbuf.h"
        "include/str.h"
        "include/math4.h"
        "include/blob_cur.h"
        "include/blob.h"
        "include/geometry.h"
        "include/color.h"
        "include/gruvbox.h"
        "include/lodge_callbacks.h" # FIXME(TS): separate lib?
        "include/membuf.h"
        "include/dynbuf.h"
        "include/frustum.h"
        "include/coordinate_systems.h"
        "include/lodge_static_call.h"
        "include/lodge_bound_func.h"
)

check_function_exists(strnlen HAVE_STRNLEN)
if(HAVE_STRNLEN)
    target_compile_definitions(lodge-lib
        PUBLIC
            HAVE_STRNLEN
    )
endif()

check_function_exists(vsnprintf HAVE_VSNPRINTF)
if(HAVE_VSNPRINTF)
    target_compile_definitions(lodge-lib
        PUBLIC
            HAVE_VSNPRINTF
    )
endif()

if(MSVC14)
    check_symbol_exists(snprintf stdio.h HAVE_SNPRINTF)
else()
    check_function_exists(snprintf HAVE_SNPRINTF)
endif()

if(HAVE_SNPRINTF)
    target_compile_definitions(lodge-lib
        PUBLIC
            HAVE_SNPRINTF
    )
endif()

if(MSVC)
    target_compile_definitions(lodge-lib
        PUBLIC
            _USE_MATH_DEFINES       # QUIRK: Define M_PI on Windows.
            NOMINMAX                # QUIRK: Do not define min and max macros in Windef.h
    )
endif()


target_include_directories(lodge-lib
	PRIVATE
        "lib/perlin-noise/src"
        "lib/murmur3"
    PUBLIC
        "include/"
)

target_link_libraries(lodge-lib
    PRIVATE
        lodge-build-flags
)