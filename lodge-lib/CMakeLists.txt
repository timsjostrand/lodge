# lodge-lib

add_library(lodge-lib STATIC "")

target_sources(lodge-lib
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/lodge.c"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_time.c"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_platform.c"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_hash.c"
        "${CMAKE_CURRENT_LIST_DIR}/txt.c"
        "${CMAKE_CURRENT_LIST_DIR}/strview.c"
        "${CMAKE_CURRENT_LIST_DIR}/strbuf.c"
        "${CMAKE_CURRENT_LIST_DIR}/str.c"
        "${CMAKE_CURRENT_LIST_DIR}/math4.c"
        "${CMAKE_CURRENT_LIST_DIR}/blob_cur.c"
        "${CMAKE_CURRENT_LIST_DIR}/blob.c"
        "${CMAKE_CURRENT_LIST_DIR}/color.c"
        "${CMAKE_CURRENT_LIST_DIR}/gruvbox.c"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_callbacks.c"
        "${CMAKE_CURRENT_LIST_DIR}/membuf.c"
        "${CMAKE_CURRENT_LIST_DIR}/frustum.c"
        "${CMAKE_CURRENT_LIST_DIR}/coordinate_systems.c"

        "${CMAKE_CURRENT_LIST_DIR}/lib/murmur3/murmur3.h"
        "${CMAKE_CURRENT_LIST_DIR}/lib/murmur3/murmur3.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/lodge.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_time.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_assert.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_platform.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_hash.h"
        "${CMAKE_CURRENT_LIST_DIR}/log.h"
        "${CMAKE_CURRENT_LIST_DIR}/txt.h"
        "${CMAKE_CURRENT_LIST_DIR}/strview.h"
        "${CMAKE_CURRENT_LIST_DIR}/strbuf.h"
        "${CMAKE_CURRENT_LIST_DIR}/str.h"
        "${CMAKE_CURRENT_LIST_DIR}/math4.h"
        "${CMAKE_CURRENT_LIST_DIR}/blob_cur.h"
        "${CMAKE_CURRENT_LIST_DIR}/blob.h"
        "${CMAKE_CURRENT_LIST_DIR}/geometry.h"
        "${CMAKE_CURRENT_LIST_DIR}/color.h"
        "${CMAKE_CURRENT_LIST_DIR}/gruvbox.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_callbacks.h" # FIXME(TS): separate lib?
        "${CMAKE_CURRENT_LIST_DIR}/membuf.h"
        "${CMAKE_CURRENT_LIST_DIR}/frustum.h"
        "${CMAKE_CURRENT_LIST_DIR}/coordinate_systems.h"
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
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-lib
    PRIVATE
        lodge-build-flags
)