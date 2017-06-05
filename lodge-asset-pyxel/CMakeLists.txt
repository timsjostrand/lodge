# lodge-asset-pyxel

add_library(lodge-asset-pyxel "")

target_sources(lodge-asset-pyxel
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/pyxel.c"
        "${CMAKE_CURRENT_LIST_DIR}/pyxel_asset.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/pyxel.h"
        "${CMAKE_CURRENT_LIST_DIR}/pyxel_asset.h"
)

target_include_directories(lodge-asset-pyxel
    PRIVATE
        "${ENGINE_INCLUDES}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-asset-pyxel
    PUBLIC
        ${LIBARCHIVE_LIBRARY}
        ${CJSON_LIBRARY}
)