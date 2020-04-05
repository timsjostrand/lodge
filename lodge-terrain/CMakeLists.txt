# lodge-terrain

add_library(lodge-terrain STATIC "")

target_sources(lodge-terrain
    PRIVATE
        #"${CMAKE_CURRENT_SOURCE_DIR}/terrain.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/quadtree.c"
    PUBLIC
        #"${CMAKE_CURRENT_SOURCE_DIR}/terrain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/quadtree.h"
)

target_include_directories(lodge-terrain
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-terrain
    lodge-lib
    lodge-collections
)
