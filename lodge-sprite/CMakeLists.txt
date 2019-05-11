# lodge-sprite

add_library(lodge-sprite STATIC "")

target_sources(lodge-sprite
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/basic_sprite.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/basic_sprite.h"
)

target_include_directories(lodge-sprite
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-sprite
    lodge-graphics
)
