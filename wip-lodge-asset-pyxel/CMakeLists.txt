# lodge-asset-pyxel

add_library(lodge-asset-pyxel STATIC "")

target_sources(lodge-asset-pyxel
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/pyxel.c"
        "${CMAKE_CURRENT_LIST_DIR}/pyxel_asset.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/pyxel.h"
        "${CMAKE_CURRENT_LIST_DIR}/pyxel_asset.h"
)

target_include_directories(lodge-asset-pyxel
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-asset-pyxel
    PRIVATE
        lodge-build-flags
        lodge-image
        lodge-gfx
        lodge-sprite
        lodge-asset-atlas
        lodge-vfs-reload # FIXME(TS)
    PUBLIC
        ${LIBARCHIVE_LIBRARY}
        ${CJSON_LIBRARY}
)