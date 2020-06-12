# lodge-sprite

add_library(lodge-sprite STATIC "")

target_sources(lodge-sprite
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/basic_sprite.c"
        "${CMAKE_CURRENT_LIST_DIR}/spritebatch.c"
        "${CMAKE_CURRENT_LIST_DIR}/animatedsprites.c"
        "${CMAKE_CURRENT_LIST_DIR}/sprite_group.c"
        "${CMAKE_CURRENT_LIST_DIR}/particles.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/basic_sprite.h"
        "${CMAKE_CURRENT_LIST_DIR}/spritebatch.h"
        "${CMAKE_CURRENT_LIST_DIR}/animatedsprites.h"
        "${CMAKE_CURRENT_LIST_DIR}/sprite_group.h"
        "${CMAKE_CURRENT_LIST_DIR}/particles.h"
)

target_include_directories(lodge-sprite
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-sprite
    PRIVATE
        lodge-build-flags
    PUBLIC
        lodge-renderer
        lodge-asset-atlas
)
