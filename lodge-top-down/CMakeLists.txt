# lodge-top-down

add_library(lodge-top-down STATIC "")

target_sources(lodge-top-down
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tiles.c"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/tiles.h"
)

target_include_directories(lodge-top-down
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
